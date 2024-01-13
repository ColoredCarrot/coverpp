//#import "D:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\DIA SDK\bin\amd64\msdia140.dll"

#include <print>
#include <iostream>
#include <filesystem>

#define NOMINMAX

#include <wil/com.h>
#include <dia2.h>
#include <psapi.h>

int exec(std::convertible_to<std::string_view> auto&& ... parts) {
    std::string s;
    (s.append(parts).append(" "), ...);
    return std::system(s.c_str());
}

using DllGetClassObject_t = HRESULT(_In_ REFCLSID rclsid, _In_ REFIID riid, _Outptr_ LPVOID FAR* ppv);

std::string bstr_to_utf8_string(BSTR bs) {
    // See https://stackoverflow.com/questions/6284524/bstr-to-stdstring-stdwstring-and-vice-versa

    const std::size_t num_wchars{SysStringLen(bs)};

    // This is not an optimization, but required, since WideCharToMultiByte returns 0 to indicate an error
    if (num_wchars == 0) {
        return {};
    }

    //TODO check if num_wchars > max int
    const int num_bytes{WideCharToMultiByte(
        CP_UTF8, 0, bs, static_cast<int>(num_wchars), nullptr, 0, nullptr, nullptr
    )};
    THROW_LAST_ERROR_IF(num_bytes == 0);

    std::string utf8(num_bytes, '\0');
    THROW_LAST_ERROR_IF(!WideCharToMultiByte(
        CP_UTF8, 0, bs, static_cast<int>(num_wchars), utf8.data(), num_bytes, nullptr, nullptr
    ));

    return utf8;
}

wil::unique_hmodule load_library(const std::filesystem::path& path) {
    return wil::unique_hmodule(THROW_LAST_ERROR_IF_NULL(LoadLibraryW(path.c_str())));
}

DllGetClassObject_t* get_DllGetClassObject_proc(const wil::unique_hmodule& library) {
    return reinterpret_cast<DllGetClassObject_t*>(THROW_LAST_ERROR_IF_NULL(
        GetProcAddress(library.get(), "DllGetClassObject")));
}

wil::com_ptr<IDiaDataSource> get_dia_data_source(const wil::unique_hmodule& dll) {
    /*
     * High-level procedure:
     *  1. Call DllGetClassObject with IID_IClassFactory
     *  2. Call cf->CreateInstance
     * See https://stackoverflow.com/a/2187454
     */

    const auto DllGetClassObject_proc = get_DllGetClassObject_proc(dll);

    wil::com_ptr<IClassFactory> cf;
    THROW_IF_FAILED(DllGetClassObject_proc(CLSID_DiaSource, IID_IClassFactory, cf.put_void()));

    wil::com_ptr<IDiaDataSource> dia_data_source;
    cf->CreateInstance(nullptr, IID_IDiaDataSource, dia_data_source.put_void());

    return dia_data_source;
}

__declspec(noinline) std::intptr_t get_instruction_pointer() {
    return reinterpret_cast<std::intptr_t>(_ReturnAddress());
}


template<typename TItem, typename TEnum>
void iterate_enum(const wil::com_ptr<TEnum>& enum_ptr, auto&& f) {
    wil::com_ptr<TItem> item;
    ULONG celt;
    while (THROW_IF_FAILED(enum_ptr->Next(1, item.put(), &celt)), celt == 1) {
        f(item);
    }
}


template<typename T>
std::string get_string(const wil::com_ptr<T>& com, HRESULT (T::* f)(BSTR*)) {
    BSTR bs;
    THROW_IF_FAILED(((*com).*f)(&bs));
    return bstr_to_utf8_string(bs);
}

template<std::integral V, typename T>
DWORD get_dword(const wil::com_ptr<T>& com, HRESULT (T::* f)(V*)) {
    V v;
    THROW_IF_FAILED(((*com).*f)(&v));
    return v;
}

template<>
struct std::formatter<IDiaEnumLineNumbers> {
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

    auto format(IDiaEnumLineNumbers& line_numbers, std::format_context& ctx) const {
        auto out = ctx.out();

        bool any = false;
        wil::com_ptr<IDiaLineNumber> line_number;
        ULONG celt;
        while (THROW_IF_FAILED(line_numbers.Next(1, line_number.put(), &celt)), celt == 1) {
            DWORD n;
            THROW_IF_FAILED(line_number->get_lineNumber(&n));

            wil::com_ptr<IDiaSourceFile> src_file;
            THROW_IF_FAILED(line_number->get_sourceFile(src_file.put()));

            out = std::format_to(out, "{}:{}\n", get_string(src_file, &IDiaSourceFile::get_fileName), n);

            any = true;
        }

        if (!any) {
            out = std::format_to(out, "(None)");
        }

        return out;
    }
};

template<typename T>
struct std::formatter<wil::com_ptr<T>> {
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

    auto format(const wil::com_ptr<T>& p, std::format_context& ctx) const {
        return std::format_to(ctx.out(), "{}", *p);
    }
};

std::intptr_t GetBaseAddress(const HANDLE hProcess) {
    if (hProcess == NULL)
        return NULL; // No access to the process

    HMODULE lphModule[1024]; // Array that receives the list of module handles
    DWORD lpcbNeeded(NULL); // Output of EnumProcessModules, giving the number of bytes requires to store all modules handles in the lphModule array

    if (!EnumProcessModules(hProcess, lphModule, sizeof(lphModule), &lpcbNeeded))
        return NULL; // Impossible to read modules

    TCHAR szModName[MAX_PATH];
    if (!GetModuleFileNameEx(hProcess, lphModule[0], szModName, sizeof(szModName) / sizeof(TCHAR)))
        return NULL; // Impossible to get module info

    const auto hmodule = lphModule[0]; // Module 0 is apparently always the EXE itself, returning its address

    MODULEINFO  info;
    THROW_LAST_ERROR_IF(!GetModuleInformation(hProcess, hmodule, &info, sizeof(info)));

    return  (std::intptr_t) info.lpBaseOfDll;
}

void read_pdb(const std::filesystem::path& path) {
    auto dll = load_library("msdia140.dll");

    auto dia_data_source = get_dia_data_source(dll);

    THROW_IF_FAILED(dia_data_source->loadDataFromPdb(path.c_str()));

    wil::com_ptr<IDiaSession> dia_session;
    THROW_IF_FAILED(dia_data_source->openSession(dia_session.put()));


    wil::com_ptr<IDiaSymbol> global_scope;
    THROW_IF_FAILED(dia_session->get_globalScope(global_scope.put()));

    wil::com_ptr<IDiaEnumSymbols> functions;
    THROW_IF_FAILED(global_scope->findChildren(SymTagFunction, L"read_pdb", nsCaseSensitive, functions.put()));

    /*
     * VA == RVA == (&the_func - GetBaseAddress())
     *
     * So we can get source file + line numbers from an instruction pointer (+ process handle)
     */

    iterate_enum<IDiaSymbol>(functions, [&](const wil::com_ptr<IDiaSymbol>& function) {
        std::println("func {}", get_string(function, &IDiaSymbol::get_name));
        std::println("VA:\t{:x}", get_dword(function, &IDiaSymbol::get_virtualAddress));
        std::println("RVA:\t{:x}", get_dword(function, &IDiaSymbol::get_relativeVirtualAddress));
    });


    std::println("relative function address: {:x}", reinterpret_cast<std::intptr_t >(&read_pdb) - GetBaseAddress(GetCurrentProcess()));



    wil::com_ptr<IDiaEnumLineNumbers> line_numbers;
//    THROW_IF_FAILED(dia_session->findLinesByRVA(get_instruction_pointer() - reinterpret_cast<std::intptr_t>(&read_pdb), 10, line_numbers.put()));
    THROW_IF_FAILED(dia_session->findLinesByVA(0xed840, 10, line_numbers.put()));
    std::println("{}", line_numbers);
}

struct CoInitializeGuard {
    CoInitializeGuard() {
        THROW_IF_FAILED(CoInitialize(nullptr));
    }

    ~CoInitializeGuard() {
        CoUninitialize();
    }
};

int main() {
    try {
        CoInitializeGuard guard;

        read_pdb(
            R"(G:\Voidev\Official\Projects\C++\Cover++\cmake-build-debug-visual-studio\Debug\coverpp.pdb)");

    } catch (const wil::ResultException& ex) {
        std::println(std::cerr, "Windows Exception: {}", ex.what());
    }

    return 0;

    std::string cmake = "cmake";

    std::filesystem::path project_dir = R"(G:\Voidev\Official\Projects\C++\Cover++\example-sut)";

    if (exec(cmake, "--version") != 0) {
        std::println(std::cerr, "CMake not found");
        return 1;
    }

    exec(cmake, "-S", project_dir.string(), "-B", "covercpp-work", "-G \"Visual Studio 17 2022\"");
    exec(cmake, "--build", "covercpp-work");

    // before and after every jump (conditional or not), insert a breakpoint instruction

    return 0;
}
