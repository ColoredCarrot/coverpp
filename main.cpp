#include "src/BreakpointDriver.hpp"
#include "src/com_utils.hpp"
#include "src/CoverageSink.hpp"

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

int exec(std::convertible_to<std::string_view> auto&& ... parts)
{
    std::string s;
    (s.append(parts).append(" "), ...);
    return std::system(s.c_str());
}

using DllGetClassObject_t = HRESULT(_In_ REFCLSID rclsid, _In_ REFIID riid, _Outptr_ LPVOID FAR* ppv);

wil::unique_hmodule load_library(const std::filesystem::path& path)
{
    return wil::unique_hmodule(THROW_LAST_ERROR_IF_NULL(LoadLibraryW(path.c_str())));
}

DllGetClassObject_t* get_DllGetClassObject_proc(const wil::unique_hmodule& library)
{
    return reinterpret_cast<DllGetClassObject_t*>(THROW_LAST_ERROR_IF_NULL(
        GetProcAddress(library.get(), "DllGetClassObject")));
}

wil::com_ptr<IDiaDataSource> get_dia_data_source(const wil::unique_hmodule& dll)
{
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


template<typename TItem, typename TEnum>
void iterate_enum(TEnum& enumeration, auto&& f)
{
    wil::com_ptr<TItem> item;
    ULONG celt;
    while (THROW_IF_FAILED(enumeration.Next(1, item.put(), &celt)), celt == 1)
    {
        f(*item);
    }
}


template<typename T>
std::string get_string(const wil::com_ptr<T>& com, HRESULT (T::* f)(BSTR*))
{
    BSTR bs;
    THROW_IF_FAILED(((*com).*f)(&bs));
    return coverpp::detail::windows::bstr_to_utf8_string(bs);
}

template<std::integral V, typename T>
V get_dword(const wil::com_ptr<T>& com, HRESULT (T::* f)(V*))
{
    V v;
    THROW_IF_FAILED(((*com).*f)(&v));
    return v;
}


template<>
struct std::formatter<IDiaEnumLineNumbers>
{
    constexpr auto parse(std::format_parse_context& ctx)
    { return ctx.begin(); }

    auto format(IDiaEnumLineNumbers& line_numbers, std::format_context& ctx) const
    {
        auto out = ctx.out();

        bool any = false;
        wil::com_ptr<IDiaLineNumber> line_number;
        ULONG celt;
        while (THROW_IF_FAILED(line_numbers.Next(1, line_number.put(), &celt)), celt == 1)
        {
            DWORD n;
            THROW_IF_FAILED(line_number->get_lineNumber(&n));

            wil::com_ptr<IDiaSourceFile> src_file;
            THROW_IF_FAILED(line_number->get_sourceFile(src_file.put()));

            out = std::format_to(out, "{}:{}\n", get_string(src_file, &IDiaSourceFile::get_fileName), n);

            any = true;
        }

        if (!any)
        {
            out = std::format_to(out, "(None)");
        }

        return out;
    }
};


#define CONCAT(a, b) a##b
#define CONCAT2(a, b) CONCAT(a, b)

#define COVERPP_FOR_EACH_COM_ITEM_(TItem, item, in_enum, celt) \
    wil::com_ptr<TItem> item; DWORD celt; \
    while (THROW_IF_FAILED((in_enum).Next(1, item.put(), &celt)), celt == 1)

#define COVERPP_FOR_EACH_COM_ITEM(TItem, item, in_enum) COVERPP_FOR_EACH_COM_ITEM_(TItem, item, in_enum, CONCAT2(celt, __LINE__))

wil::com_ptr<IDiaEnumSourceFiles> get_enum_source_files(IDiaSession& dia_session)
{
    // See https://learn.microsoft.com/en-us/visualstudio/debugger/debug-interface-access/idiaenumsourcefiles?view=vs-2022

    wil::com_ptr<IDiaEnumTables> dia_tables;
    THROW_IF_FAILED(dia_session.getEnumTables(dia_tables.put()));

    wil::com_ptr<IDiaEnumSourceFiles> dia_source_files;
    COVERPP_FOR_EACH_COM_ITEM(IDiaTable, dia_table, *dia_tables)
    {
        const HRESULT hr = dia_table->QueryInterface(IID_IDiaEnumSourceFiles, dia_source_files.put_void());
        if (hr == S_OK)
        {
            break;
        }
        (void) dia_source_files.detach();
    }

    return dia_source_files;
}

bool path_is_subpath_of(const std::filesystem::path& sub_path, const std::filesystem::path& base_path)
{
    const auto r = std::ranges::mismatch(base_path, sub_path);
    return r.in1 == base_path.end();
}

coverpp::CoverageSink
get_project_source_lines(const std::filesystem::path& project_dir, IDiaSession& dia_session)
{
    auto dia_source_files = get_enum_source_files(dia_session);

    coverpp::CoverageSink sink;
    COVERPP_FOR_EACH_COM_ITEM(IDiaSourceFile, dia_source_file, *dia_source_files)
    {
        std::filesystem::path file = get_string(dia_source_file, &IDiaSourceFile::get_fileName);
        if (!path_is_subpath_of(file, project_dir))
        {
            continue;
        }

        wil::com_ptr<IDiaEnumSymbols> dia_enum_compilands;
        THROW_IF_FAILED(dia_source_file->get_compilands(dia_enum_compilands.put()));

        COVERPP_FOR_EACH_COM_ITEM(IDiaSymbol, dia_compiland, *dia_enum_compilands)
        {
            wil::com_ptr<IDiaEnumLineNumbers> dia_enum_line_numbers;
            THROW_IF_FAILED(
                dia_session.findLines(dia_compiland.get(), dia_source_file.get(), dia_enum_line_numbers.put()));

            COVERPP_FOR_EACH_COM_ITEM(IDiaLineNumber, dia_line_number, *dia_enum_line_numbers)
            {
                sink.track_coverage(
                    file,
                    {
                        .lineBegin = get_dword(dia_line_number, &IDiaLineNumber::get_lineNumber),
                        .lineEnd = get_dword(dia_line_number, &IDiaLineNumber::get_lineNumberEnd),
                        .columnBegin = get_dword(dia_line_number, &IDiaLineNumber::get_columnNumber),
                        .columnEnd = get_dword(dia_line_number, &IDiaLineNumber::get_columnNumberEnd),
                    }
                );
            }
        }
    }

    return sink;
}


std::intptr_t get_base_address(HANDLE process)
{
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


std::optional<std::filesystem::path> get_file_by_line_numbers(IDiaEnumLineNumbers& line_numbers)
{
    THROW_IF_FAILED(line_numbers.Reset());

    DWORD celt;
    wil::com_ptr<IDiaLineNumber> line_number;
    THROW_IF_FAILED(line_numbers.Next(1, line_number.put(), &celt));
    if (celt != 1)
    {
        return std::nullopt;
    }

    wil::com_ptr<IDiaSourceFile> src_file;
    THROW_IF_FAILED(line_number->get_sourceFile(src_file.put()));

    return get_string(src_file, &IDiaSourceFile::get_fileName);
}

std::optional<std::filesystem::path> get_file_by_va(unsigned long long va, IDiaSession& dia_session)
{
    wil::com_ptr<IDiaEnumLineNumbers> line_numbers;
    THROW_IF_FAILED(dia_session.findLinesByVA(va, 1, line_numbers.put()));
    return get_file_by_line_numbers(*line_numbers);
}

template<typename TItem, typename TEnum>
wil::com_ptr<TItem> get_single_item(TEnum& enumeration)
{
    LONG count;
    THROW_IF_FAILED(enumeration.get_Count(&count));
    if (count != 1)
    {
        return {};
    }

    wil::com_ptr<TItem> item;
    THROW_IF_FAILED(enumeration.Item(0, item.put()));
    return item;
}

using coverpp::VirtualAddress;
using coverpp::InstructionPointer;

int run_with_coverage(const std::filesystem::path& src_dir, const std::filesystem::path& exe,
                      const std::filesystem::path& pdb)
{
    // Step #1: Load PDB
    auto dia_dll = load_library("msdia140.dll");
    auto dia_data_source = get_dia_data_source(dia_dll);

    THROW_IF_FAILED(dia_data_source->loadDataFromPdb(pdb.c_str()));

    wil::com_ptr<IDiaSession> dia_session;
    THROW_IF_FAILED(dia_data_source->openSession(dia_session.put()));


    auto reachable = get_project_source_lines(src_dir, *dia_session);
    std::println("reachable: {}", reachable);


    // Step #2: Find address of main function
    wil::com_ptr<IDiaSymbol> dia_global_scope;
    THROW_IF_FAILED(dia_session->get_globalScope(dia_global_scope.put()));

    wil::com_ptr<IDiaEnumSymbols> dia_enum_main;
    THROW_IF_FAILED(dia_global_scope->findChildren(
        SymTagEnum::SymTagFunction, L"main", nsfCaseSensitive, dia_enum_main.put()
    ));
    auto dia_main = get_single_item<IDiaSymbol>(*dia_enum_main);
    if (!dia_main)
    {
        throw std::runtime_error("Could not find main function in PDB");
    }

    const VirtualAddress main_entry_va{get_dword(dia_main, &IDiaSymbol::get_virtualAddress)};


    coverpp::CoverageSink sink;


    // Step #3: Run the exe in a new process with coverage tracking

    // See this amazingly helpful resource: https://www.codeproject.com/Articles/43682/Writing-a-basic-Windows-debugger

    STARTUPINFOW si{};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi{};
    // Inherit env and workdir from the coverage process
    THROW_LAST_ERROR_IF(!CreateProcessW(
        exe.c_str(), nullptr, nullptr, nullptr, false, DEBUG_ONLY_THIS_PROCESS, nullptr, nullptr, &si, &pi
    ));

    const HANDLE hProcess = pi.hProcess;

    // Step #4: Set breakpoint in main function
    coverpp::BreakpointDriver breakpoint_driver{hProcess};

    std::unordered_map<DWORD, HANDLE> thread_handles;

    int exit_code = 0;
    bool first_breakpoint = true;
    while (true)
    {
        DEBUG_EVENT evt;
        THROW_LAST_ERROR_IF_NOT(WaitForDebugEventEx(&evt, INFINITE));

        bool is_exit = false;
        DWORD continue_status = DBG_CONTINUE;
        switch (evt.dwDebugEventCode)
        {
        case CREATE_PROCESS_DEBUG_EVENT:
        {
            thread_handles.emplace(evt.dwThreadId, evt.u.CreateProcessInfo.hThread);
            breakpoint_driver.set_base_address(
                InstructionPointer{(std::uintptr_t) evt.u.CreateProcessInfo.lpBaseOfImage});
            breakpoint_driver.set_breakpoint(breakpoint_driver.va_to_ip(main_entry_va));
            break;
        }
        case CREATE_THREAD_DEBUG_EVENT:
        {
            thread_handles.emplace(evt.dwThreadId, evt.u.CreateThread.hThread);
            break;
        }
        case OUTPUT_DEBUG_STRING_EVENT:
        {
            //TODO unicode handling, also nDebugStringLength might be too small, so can't use that (at least not only)
            auto buf = std::make_unique<char[]>(evt.u.DebugString.nDebugStringLength);
            THROW_LAST_ERROR_IF_NOT(ReadProcessMemory(hProcess, evt.u.DebugString.lpDebugStringData, buf.get(),
                                                      evt.u.DebugString.nDebugStringLength,
                                                      nullptr));

            OutputDebugString(buf.get());
            std::println("Received debug string: {}", buf.get());
            break;
        }
        case EXIT_PROCESS_DEBUG_EVENT:
        {
            is_exit = true;
            exit_code = static_cast<int>(evt.u.ExitProcess.dwExitCode);
            std::println("Process finished with exit code {}", exit_code);
            break;
        }
        case EXCEPTION_DEBUG_EVENT:
        {
            const auto hThread = thread_handles.at(evt.dwThreadId);

            const InstructionPointer ip{(std::uintptr_t) evt.u.Exception.ExceptionRecord.ExceptionAddress};
            const auto va = breakpoint_driver.ip_to_va(ip);

            if (evt.u.Exception.ExceptionRecord.ExceptionCode == STATUS_BREAKPOINT)
            {
                if (first_breakpoint)
                {
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
                --context.Rip; // Decrement because we get here *after* the INT3 instruction executed
//                context.Dr6 |= 1 << 14;
                THROW_LAST_ERROR_IF_NOT(SetThreadContext(hThread, &context));

                breakpoint_driver.remove_breakpoint(ip);

                continue_status = DBG_EXCEPTION_HANDLED;
            }
            else if (evt.u.Exception.ExceptionRecord.ExceptionCode == STATUS_SINGLE_STEP)
            {
                wil::com_ptr<IDiaEnumLineNumbers> line_numbers;
                THROW_IF_FAILED(dia_session->findLinesByVA(va.value, 1, line_numbers.put()));

                std::println("single st {:x}", ip.value);
                std::println("single step VA: {} @ {}", va, *line_numbers);

                const auto file = get_file_by_line_numbers(*line_numbers);

                // Note: Gets into infinite loop in some external Windows file without this check
                if (!file || path_is_subpath_of(*file, src_dir))
                {
                    // FUCK YES, THIS IS WORKING!
                    if (file)
                    {
                        auto line_number = get_single_item<IDiaLineNumber>(*line_numbers);
                        sink.track_coverage(
                            *file,
                            {
                                .lineBegin = get_dword(line_number, &IDiaLineNumber::get_lineNumber),
                                .lineEnd = get_dword(line_number, &IDiaLineNumber::get_lineNumberEnd),
                                .columnBegin = get_dword(line_number, &IDiaLineNumber::get_columnNumber),
                                .columnEnd = get_dword(line_number, &IDiaLineNumber::get_columnNumberEnd),
                            }
                        );
                    }

                    CONTEXT context{};
                    context.ContextFlags = CONTEXT_CONTROL;
                    THROW_LAST_ERROR_IF_NOT(GetThreadContext(hThread, &context));
                    context.EFlags |= (1 << 8) /*| (1 << 16)*/;
                    THROW_LAST_ERROR_IF_NOT(SetThreadContext(hThread, &context));
                }


                continue_status = DBG_EXCEPTION_HANDLED;
            }
            else
            {
                std::println("exc");
                continue_status = DBG_EXCEPTION_NOT_HANDLED;
            }

            break;
        }
        case RIP_EVENT:
        {
            std::println("RIP! {} - {}", evt.u.RipInfo.dwError, evt.u.RipInfo.dwType);
            is_exit = true;
            exit_code = 99;
            break;
        }
        }

        THROW_LAST_ERROR_IF_NOT(ContinueDebugEvent(evt.dwProcessId, evt.dwThreadId, continue_status));

        if (is_exit)
        {
            break;
        }
    }

    std::println("reached: {}", sink);

    return exit_code;
}

struct CoInitializeGuard
{
    CoInitializeGuard()
    {
        THROW_IF_FAILED(CoInitialize(nullptr));
    }

    ~CoInitializeGuard()
    {
        CoUninitialize();
    }
};

int main()
{
    try
    {
        CoInitializeGuard guard;

        return run_with_coverage(
            R"(G:\Voidev\Official\Projects\C++\Cover++)",
            R"(G:\Voidev\Official\Projects\C++\Cover++\cmake-build-debug-visual-studio\example-sut\Debug\example-sut.exe)",
            R"(G:\Voidev\Official\Projects\C++\Cover++\cmake-build-debug-visual-studio\example-sut\Debug\example-sut.pdb)"
        );
    } catch (const wil::ResultException& ex)
    {
        std::println(std::cerr, "Windows Exception: {}", ex.what());
    }

    return 0;

    std::string cmake = "cmake";

    std::filesystem::path project_dir = R"(G:\Voidev\Official\Projects\C++\Cover++\example-sut)";

    if (exec(cmake, "--version") != 0)
    {
        std::println(std::cerr, "CMake not found");
        return 1;
    }

    exec(cmake, "-S", project_dir.string(), "-B", "covercpp-work", "-G \"Visual Studio 17 2022\"");
    exec(cmake, "--build", "covercpp-work");

    // before and after every jump (conditional or not), insert a breakpoint instruction

    return 0;
}
