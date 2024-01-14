//#import "D:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\DIA SDK\bin\amd64\msdia140.dll"

#include <print>
#include <iostream>
#include <filesystem>
#include <cassert>
#include <unordered_map>

#define NOMINMAX

#include <wil/com.h>
#include <dia2.h>
#include <psapi.h>
#include <intrin.h>

#define THROW_LAST_ERROR_IF_NOT(x) THROW_LAST_ERROR_IF(!(x))

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

std::intptr_t get_base_address(HANDLE process) {
    assert(process);

    HMODULE lphModule[1024]; // Array that receives the list of module handles
    DWORD lpcbNeeded(
        NULL); // Output of EnumProcessModules, giving the number of bytes requires to store all modules handles in the lphModule array

    if (!EnumProcessModules(process, lphModule, sizeof(lphModule), &lpcbNeeded))
        return NULL; // Impossible to read modules

    TCHAR szModName[MAX_PATH];
    if (!GetModuleFileNameEx(process, lphModule[0], szModName, sizeof(szModName) / sizeof(TCHAR)))
        return NULL; // Impossible to get module info

    const auto hmodule = lphModule[0]; // Module 0 is apparently always the EXE itself, returning its address

    MODULEINFO info;
    THROW_LAST_ERROR_IF(!GetModuleInformation(process, hmodule, &info, sizeof(info)));

    return (std::intptr_t) info.lpBaseOfDll;
}

std::intptr_t instruction_pointer_to_va(std::intptr_t ip, HANDLE process = GetCurrentProcess()) {
    assert(process);
    return ip - get_base_address(process);
}

std::optional<std::filesystem::path> get_file_by_line_numbers(IDiaEnumLineNumbers& line_numbers) {
    DWORD celt;
    wil::com_ptr<IDiaLineNumber> line_number;
    THROW_IF_FAILED(line_numbers.Next(1, line_number.put(), &celt));
    if (celt != 1) {
        return std::nullopt;
    }

    wil::com_ptr<IDiaSourceFile> src_file;
    THROW_IF_FAILED(line_number->get_sourceFile(src_file.put()));

    return get_string(src_file, &IDiaSourceFile::get_fileName);
}

std::optional<std::filesystem::path> get_file_by_va(unsigned long long va, IDiaSession& dia_session) {
    wil::com_ptr<IDiaEnumLineNumbers> line_numbers;
    THROW_IF_FAILED(dia_session.findLinesByVA(va, 1, line_numbers.put()));
    return get_file_by_line_numbers(*line_numbers);
}

bool path_is_subpath_of(const std::filesystem::path& sub_path, const std::filesystem::path& base_path) {
    const auto r = std::ranges::mismatch(base_path, sub_path);
    return r.in1 == base_path.end();
}

int run_with_coverage(const std::filesystem::path& src_dir, const std::filesystem::path& exe,
                      const std::filesystem::path& pdb) {
    // Step #1: Load PDB
    auto dia_dll = load_library("msdia140.dll");
    auto dia_data_source = get_dia_data_source(dia_dll);

    THROW_IF_FAILED(dia_data_source->loadDataFromPdb(pdb.c_str()));

    wil::com_ptr<IDiaSession> dia_session;
    THROW_IF_FAILED(dia_data_source->openSession(dia_session.put()));


    // Step #2: Run the exe in a new process with coverage tracking

    // See this amazingly helpful resource: https://www.codeproject.com/Articles/43682/Writing-a-basic-Windows-debugger

    STARTUPINFOW si{};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi{};
    // Inherit env and workdir from the coverage process
    THROW_LAST_ERROR_IF(!CreateProcessW(
        exe.c_str(), nullptr, nullptr, nullptr, false, DEBUG_ONLY_THIS_PROCESS, nullptr, nullptr, &si, &pi
    ));

    const HANDLE hProcess = pi.hProcess;

    std::unordered_map<DWORD, HANDLE> thread_handles;

    int exit_code = 0;
    bool first_breakpoint = true;
    while (true) {
        DEBUG_EVENT evt;
        THROW_LAST_ERROR_IF_NOT(WaitForDebugEventEx(&evt, INFINITE));

        bool is_exit = false;
        DWORD continue_status = DBG_CONTINUE;
        switch (evt.dwDebugEventCode) {
        case CREATE_PROCESS_DEBUG_EVENT: {
            thread_handles.emplace(evt.dwThreadId, evt.u.CreateProcessInfo.hThread);
            break;
        }
        case CREATE_THREAD_DEBUG_EVENT: {
            thread_handles.emplace(evt.dwThreadId, evt.u.CreateThread.hThread);
            break;
        }
        case OUTPUT_DEBUG_STRING_EVENT: {
            //TODO unicode handling, also nDebugStringLength might be too small, so can't use that (at least not only)
            auto buf = std::make_unique<char[]>(evt.u.DebugString.nDebugStringLength);
            THROW_LAST_ERROR_IF_NOT(ReadProcessMemory(hProcess, evt.u.DebugString.lpDebugStringData, buf.get(),
                                                      evt.u.DebugString.nDebugStringLength,
                                                      nullptr));

            OutputDebugString(buf.get());
            std::println("Received debug string: {}", buf.get());
            break;
        }
        case EXIT_PROCESS_DEBUG_EVENT: {
            is_exit = true;
            exit_code = static_cast<int>(evt.u.ExitProcess.dwExitCode);
            std::println("Process finished with exit code {}", exit_code);
            break;
        }
        case EXCEPTION_DEBUG_EVENT: {
            const auto hThread = thread_handles.at(evt.dwThreadId);

            if (evt.u.Exception.ExceptionRecord.ExceptionCode == STATUS_BREAKPOINT) {
                if (first_breakpoint) {
                    first_breakpoint = false;
                    break;
                }

                std::println("breakpoint hit");

                // Set the TF (Trap Flag, bit 8) in the EFLAGS register.
                // When this flag is set, the processor traps after every instruction with STATUS_SINGLE_STEP.
                CONTEXT context{};
                context.ContextFlags = CONTEXT_CONTROL;
                THROW_LAST_ERROR_IF_NOT(GetThreadContext(hThread, &context));
                context.EFlags |= (1 << 8) | (1 << 16);
//                context.Dr6 |= 1 << 14;
                THROW_LAST_ERROR_IF_NOT(SetThreadContext(hThread, &context));

                continue_status = DBG_EXCEPTION_HANDLED;
            } else if (evt.u.Exception.ExceptionRecord.ExceptionCode == STATUS_SINGLE_STEP) {
                const auto ip = (std::intptr_t) evt.u.Exception.ExceptionRecord.ExceptionAddress;
                const auto va = instruction_pointer_to_va(ip, hProcess);

                std::println("single step VA: {:x}", va);

                const auto file = get_file_by_va(va, *dia_session);
                // Note: Gets into infinite loop in some external Windows file without this check
                if (file && path_is_subpath_of(*file, src_dir)) {
                    // FUCK YES, THIS IS WORKING!
                    // TODO: Track the source line, add to list of reached lines

                    CONTEXT context{};
                    context.ContextFlags = CONTEXT_CONTROL;
                    THROW_LAST_ERROR_IF_NOT(GetThreadContext(hThread, &context));
                    context.EFlags |= (1 << 8) /*| (1 << 16)*/;
                    THROW_LAST_ERROR_IF_NOT(SetThreadContext(hThread, &context));
                }


                continue_status = DBG_EXCEPTION_HANDLED;
            } else {
                std::println("exc");
                continue_status = DBG_EXCEPTION_NOT_HANDLED;
            }

            break;
        }
        case RIP_EVENT: {
            std::println("RIP! {} - {}", evt.u.RipInfo.dwError, evt.u.RipInfo.dwType);
            is_exit = true;
            exit_code = 99;
            break;
        }
        }

        THROW_LAST_ERROR_IF_NOT(ContinueDebugEvent(evt.dwProcessId, evt.dwThreadId, continue_status));

        if (is_exit) {
            break;
        }
    }

    return exit_code;
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

        return run_with_coverage(
            R"(G:\Voidev\Official\Projects\C++\Cover++)",
            R"(G:\Voidev\Official\Projects\C++\Cover++\cmake-build-debug-visual-studio\example-sut\Debug\example-sut.exe)",
            R"(G:\Voidev\Official\Projects\C++\Cover++\cmake-build-debug-visual-studio\example-sut\Debug\example-sut.pdb)"
        );
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
