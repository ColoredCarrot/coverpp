#include "run.hpp"

#include "BreakpointDriver.hpp"
#include "com_utils.hpp"
#include "CoverageSink.hpp"
#include "WindowsCoverageSession.hpp"
#include "exporter/SourceFileExporter.hpp"
#include "report/CoverageProcessor.hpp"
#include "exporter/html/HtmlExporter.hpp"
#include "seh_descriptions.hpp"
#include "util/encodings_util.hpp"
#include "exporter/raw/RawExporter.hpp"
#include "util/console_color.hpp"

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
    {
        // Impossible to read modules
        return NULL;
    }

    TCHAR szModName[MAX_PATH];
    if (!GetModuleFileNameEx(process, lphModule[0], szModName, sizeof(szModName) / sizeof(TCHAR)))
    {
        // Impossible to get module info
        return NULL;
    }

    const auto hmodule = lphModule[0]; // Module 0 is apparently always the EXE itself, returning its address

    MODULEINFO info;
    THROW_LAST_ERROR_IF(!GetModuleInformation(process, hmodule, &info, sizeof(info)));

    return (std::intptr_t) info.lpBaseOfDll;
}


using coverpp::VirtualAddress;
using coverpp::InstructionPointer;

static bool is_exit_path(const std::filesystem::path& file)
{
    // Tests for paths like minkernel\crts\ucrt\src\appcrt\startup\exit.cpp

    if (std::ranges::contains_subrange(
        file | std::ranges::to<std::vector>(),
        std::filesystem::path{"a/_work/1/s/src/vctools/crt"} | std::ranges::to<std::vector>()
    ))
    {
        return true;
    }

    if (file.has_root_directory() || file.has_root_name() || file.has_root_path())
    {
        return false;
    }

    auto it = file.begin();
    return it != file.end() && *it == "minkernel";
}

static std::string get_loaded_dll_name(HANDLE process, const LOAD_DLL_DEBUG_INFO& info)
{
    if (!info.lpImageName)
    {
        return "<unknown>";
    }

    char* ptr_in_debuggee;
    THROW_LAST_ERROR_IF_NOT(
        ReadProcessMemory(process, info.lpImageName, &ptr_in_debuggee, sizeof(ptr_in_debuggee), nullptr));

    SIZE_T len;
    char buf[512];
    (ReadProcessMemory(process, ptr_in_debuggee, buf, sizeof(buf) - 2, &len));
    buf[len] = '\0';
    buf[len + 1] = '\0';

    return info.fUnicode ? coverpp::windows::utf16le_to_utf8((const wchar_t*) buf) : std::string{(const char*) buf};
}

static std::wstring make_command_line_string(std::wstring_view module_name, std::string_view args)
{
    std::wstring s;
    s.reserve(module_name.length() + 1 + args.length());
    s.append(module_name);
    s.push_back(' ');
    s.append(coverpp::windows::utf8_to_utf16le(args));
    return s;
}


namespace coverpp
{
int run_with_coverage(const CoverageParams& params)
{
    coverpp::windows::WindowsCoverageSession coverage_session{params};

    auto reachable = coverage_session.collect_source_lines();
    if (params.verbosity >= 1)
    {
        std::println("Found {} reachable tracepoints",
                     coverpp::styled<coverpp::ColorBold::cyan>(reachable.count_tracepoints()));
    }


    // Step #2: Find address of main function
    const auto main_entry_va = coverage_session.find_entrypoint();


    coverpp::CoverageSink sink;


    // Step #3: Run the exe in a new process with coverage tracking

    // See this amazingly helpful resource: https://www.codeproject.com/Articles/43682/Writing-a-basic-Windows-debugger

    STARTUPINFOW si{};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi{};
    // Inherit env and workdir from the coverage process
    auto command_line = make_command_line_string(params.program.c_str(), params.program_args);
    THROW_LAST_ERROR_IF_NOT(CreateProcessW(
        params.program.c_str(), command_line.data(),
        nullptr, nullptr, false, DEBUG_ONLY_THIS_PROCESS, nullptr, nullptr,
        &si, &pi
    ));

    const HANDLE hProcess = pi.hProcess;

    // Step #4: Set breakpoint in main function
    coverpp::BreakpointDriver breakpoint_driver{hProcess};

    std::unordered_map<DWORD, HANDLE> thread_handles;

    // TODO: Running under ASAN is bad (what if only coverpp under ASAN, but not the SUT? <-- untested) because that generates many exceptions all the time
    //  because ASAN uses SEH under the hood, see e.g. https://github.com/catchorg/Catch2/issues/2286#issuecomment-927974627

    bool first_breakpoint = true;
    std::optional<int> exit_code;
    do
    {
        DEBUG_EVENT evt;
        THROW_LAST_ERROR_IF_NOT(WaitForDebugEventEx(&evt, INFINITE));

        DWORD continue_status = DBG_CONTINUE;
        switch (evt.dwDebugEventCode)
        {
        case CREATE_PROCESS_DEBUG_EVENT:
        {
            thread_handles.emplace(evt.dwThreadId, evt.u.CreateProcessInfo.hThread);
            breakpoint_driver.set_base_address(
                InstructionPointer{(std::uintptr_t) evt.u.CreateProcessInfo.lpBaseOfImage});
            const auto main_entry_ip = breakpoint_driver.va_to_ip(main_entry_va);

            // Set breakpoints at all reachable locations
            for (const auto& [source_file, dia_line_number] : coverage_session.enum_source_lines())
            {
                coverpp::VirtualAddress va;
                THROW_IF_FAILED(dia_line_number.get_virtualAddress(&va.value));
                const auto ip = breakpoint_driver.va_to_ip(va);

                breakpoint_driver.set_breakpoint(ip);

                if (params.verbosity >= 5)
                {
                    std::println("Set breakpoint at {}", ip);
                }
            }

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
            exit_code = static_cast<int>(evt.u.ExitProcess.dwExitCode);
            std::println("Process finished with exit code {}", *exit_code);
            break;
        }
        case EXCEPTION_DEBUG_EVENT:
        {
            const auto hThread = thread_handles.at(evt.dwThreadId);

            const InstructionPointer ip{(std::uintptr_t) evt.u.Exception.ExceptionRecord.ExceptionAddress};
            const auto va = breakpoint_driver.ip_to_va(ip);

            const auto tracepoint = coverage_session.resolve_tracepoint(va);

            continue_status = DBG_EXCEPTION_NOT_HANDLED;

            if (evt.u.Exception.ExceptionRecord.ExceptionCode == STATUS_BREAKPOINT)
            {
                if (first_breakpoint)
                {
                    first_breakpoint = false;
                    continue_status = DBG_EXCEPTION_HANDLED;
                    break;
                }

                if (params.verbosity >= 5)
                {
                    std::println(
                        "Breakpoint hit at {}",
                        tracepoint
                        ? std::format("{}:{} (address {})", tracepoint->first.u8string(), tracepoint->second.lineBegin,
                                      ip)
                        : std::format("address {}", ip)
                    );
                }

                coverage_session.trace(sink, va);

                // Set the TF (Trap Flag, bit 8) in the EFLAGS register.
                // When this flag is set, the processor traps after every instruction with STATUS_SINGLE_STEP.
                CONTEXT context{};
                context.ContextFlags = CONTEXT_CONTROL;
                THROW_LAST_ERROR_IF_NOT(GetThreadContext(hThread, &context));
//                context.EFlags |= (1 << 8) | (1 << 16);
                --context.Rip; // Decrement because we get here *after* the INT3 instruction executed
                THROW_LAST_ERROR_IF_NOT(SetThreadContext(hThread, &context));

                breakpoint_driver.remove_breakpoint(ip);

                continue_status = DBG_EXCEPTION_HANDLED;
            }
            else if (evt.u.Exception.ExceptionRecord.ExceptionCode == STATUS_SINGLE_STEP)
            {
                // Note: Gets into infinite loop in some external Windows file without this check
                const auto file = coverage_session.trace(sink, va);
                if (!file || !is_exit_path(*file))
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
                const bool first_chance = evt.u.Exception.dwFirstChance;
                const auto& record = evt.u.Exception.ExceptionRecord;
                if (!first_chance || params.print_first_chance_seh_exceptions)
                {
                    std::println(
                        "{} {} encountered at {}",
                        first_chance ? "First-chance" : "Unhandled",
                        coverpp::describe_seh_exception(record.ExceptionCode, record.ExceptionInformation),
                        tracepoint ? std::format("{}:{}", tracepoint->first.u8string(), tracepoint->second.lineBegin)
                                   : std::format("{}", record.ExceptionAddress)
                    );
                }
                continue_status = DBG_EXCEPTION_NOT_HANDLED;
            }

            break;
        }
        case RIP_EVENT:
        {
            std::println("RIP! {} - {}", evt.u.RipInfo.dwError, evt.u.RipInfo.dwType);
            exit_code = 99;
            break;
        }
        case LOAD_DLL_DEBUG_EVENT:
        {
            // We need to manually release the DLL handle once we're done with it
            wil::unique_handle dll{evt.u.LoadDll.hFile};
            if (params.verbosity >= 2)
            {
                std::println("Loaded DLL: {}", get_loaded_dll_name(hProcess, evt.u.LoadDll));
            }
            break;
        }
        case UNLOAD_DLL_DEBUG_EVENT:
        case EXIT_THREAD_DEBUG_EVENT:
        {
            // Nothing to do
            break;
        }
        default:
        {
            std::println(std::cerr, "Unknown debug event code: 0x{:X}", evt.dwDebugEventCode);
            break;
        }
        }

        THROW_LAST_ERROR_IF_NOT(ContinueDebugEvent(evt.dwProcessId, evt.dwThreadId, continue_status));
    } while (!exit_code);

    if (params.verbosity >= 1)
    {
        std::println("Reached {} tracepoints",
                     coverpp::styled<coverpp::ColorBold::cyan>(sink.count_tracepoints()));
    }

    coverpp::BasicReport report = coverpp::process_coverage_sink(sink);
    coverpp::BasicReport reachable_report = coverpp::process_coverage_sink(reachable);

    /*coverpp::SourceFileExporter report_generator{params.out_dir};
    report_generator.run(report, reachable_report);

    coverpp::HtmlExporter exporter{params.out_dir};
    exporter.run(report, reachable_report);*/

    coverpp::RawExporter raw_exporter{params.out_dir / "report.coverpp", params.source_dir};
    raw_exporter.run(report, reachable_report);

    return *exit_code;
}
}
