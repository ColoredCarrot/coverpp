#include "src/BreakpointDriver.hpp"
#include "src/com_utils.hpp"
#include "src/CoverageSink.hpp"
#include "src/WindowsCoverageSession.hpp"
#include "src/report/SourceFileReportGenerator.hpp"
#include "src/report/CoverageProcessor.hpp"

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


template<typename T>
std::string get_string(const wil::com_ptr<T>& com, HRESULT (T::* f)(BSTR*))
{
    BSTR bs;
    THROW_IF_FAILED(((*com).*f)(&bs));
    return coverpp::detail::windows::bstr_to_utf8_string(bs);
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
    coverpp::windows::WindowsCoverageSession coverage_session{{src_dir, exe, pdb}};

    auto reachable = coverage_session.collect_source_lines();
    std::println("reachable: {}", reachable);


    // Step #2: Find address of main function
    const auto main_entry_va = coverage_session.find_entrypoint();


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
                std::println("single step {:x}", ip.value);

                // Note: Gets into infinite loop in some external Windows file without this check
                if (coverage_session.trace(sink, va))
                {
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

    coverpp::SourceFileReportGenerator report_generator{
        R"(G:\Voidev\Official\Projects\C++\Cover++\cmake-build-debug-visual-studio\Debug\covercpp-work\report)"};

    report_generator.generate_report(coverpp::process_coverage_sink(sink), coverpp::process_coverage_sink(reachable));

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
