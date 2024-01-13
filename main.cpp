//#import "D:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\DIA SDK\bin\amd64\msdia140.dll"

#include <print>
#include <iostream>
#include <filesystem>

#define NOMINMAX

#include <wil/com.h>
#include <dia2.h>

int exec(std::convertible_to<std::string_view> auto &&... parts) {
    std::string s;
    (s.append(parts).append(" "), ...);
    return std::system(s.c_str());
}

using DllGetClassObject_t = HRESULT(_In_ REFCLSID rclsid, _In_ REFIID riid, _Outptr_ LPVOID FAR *ppv);

std::string bstr_to_utf8_string(BSTR bs) {
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

void read_pdb(const std::filesystem::path &path) {
    const std::filesystem::path dll = "msdia140.dll";
    if (!std::filesystem::exists(dll)) {
        throw std::runtime_error("DLL not found");
    }

    wil::unique_hmodule h(LoadLibraryW(dll.c_str()));
    if (!h) {
        throw std::runtime_error("DLL not loaded");
    }

    const auto DllGetClassObject_proc = (DllGetClassObject_t *) THROW_LAST_ERROR_IF_NULL(
        GetProcAddress(h.get(), "DllGetClassObject"));

    wil::com_ptr<IClassFactory> cf;
    THROW_IF_FAILED(DllGetClassObject_proc(CLSID_DiaSource, IID_IClassFactory, cf.put_void()));

    wil::com_ptr<IDiaDataSource> dia_data_source;
    cf->CreateInstance(nullptr, IID_IDiaDataSource, dia_data_source.put_void());

    THROW_IF_FAILED(dia_data_source->loadDataFromPdb(path.c_str()));

    wil::com_ptr<IDiaSession> dia_session;
    THROW_IF_FAILED(dia_data_source->openSession(dia_session.put()));

    wil::com_ptr<IDiaSymbol> dia_global_scope;
    THROW_IF_FAILED(dia_session->get_globalScope(dia_global_scope.put()));

    wil::com_ptr<IDiaEnumTables> dia_tables;
    THROW_IF_FAILED(dia_session->getEnumTables(dia_tables.put()));

    wil::com_ptr<IDiaTable> dia_table;
    ULONG celt;
    while (THROW_IF_FAILED(dia_tables->Next(1, dia_table.put(), &celt)), celt == 1) {
        BSTR name_bs;
        dia_table->get_name(&name_bs);
        std::println("Symbol: {}", bstr_to_utf8_string(name_bs));
    }
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
            R"(G:\Voidev\Official\Projects\C++\Cover++\cmake-build-debug-visual-studio\Debug\covercpp-work\Debug\example-sut.pdb)");

    } catch (const wil::ResultException &ex) {
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
