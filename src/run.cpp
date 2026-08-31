#include "run.hpp"

#include "BreakpointDriver.hpp"
#include "com_utils.hpp"
#include "CoverageSink.hpp"
#include "WindowsCoverageSession.hpp"
#include "report/CoverageProcessor.hpp"
#include "seh_descriptions.hpp"
#include "util/encodings_util.hpp"
#include "exporter/raw/RawExporter.hpp"
#include "util/console_color.hpp"
#include "util/guard.hpp"

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

            out = std::format_to(out, "{}:{}\n", coverpp::windows::detail::get_string(src_file, &IDiaSourceFile::get_fileName), n);

            any = true;
        }

        if (!any)
        {
            out = std::format_to(out, "(None)");
        }

        return out;
    }
};


using coverpp::VirtualAddress;

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

	if (!ptr_in_debuggee)
	{
        return "<unknown>";
	}

    SIZE_T len{};
    char buf[512];
    (ReadProcessMemory(process, ptr_in_debuggee, buf, sizeof(buf) - 2, &len));
    buf[len] = '\0';
    buf[len + 1] = '\0';

    return info.fUnicode ? coverpp::windows::utf16le_to_utf8((const wchar_t*) buf) : std::string{(const char*) buf};
}

static bool contains_space_or_tab(std::wstring_view s)
{
	return s.find_first_of(L" \t") != std::wstring_view::npos;
}

static void append_argument(std::wstring& out, std::wstring_view arg)
{
	bool const quoted = arg.empty() || contains_space_or_tab(arg);

	if (quoted)
	{
		out.push_back(L'"');
	}

	std::size_t backslashes = 0;

	for (wchar_t const ch : arg)
	{
		if (ch == L'\\')
		{
			++backslashes;
			continue;
		}

		if (ch == L'"')
		{
			// A literal quote requires:
			//   N backslashes -> 2*N + 1 backslashes, then the quote.
			out.append(backslashes * 2 + 1, L'\\');
			out.push_back(L'"');
			backslashes = 0;
			continue;
		}

		// Backslashes not immediately preceding a quote are literal.
		out.append(backslashes, L'\\');
		backslashes = 0;
		out.push_back(ch);
	}

	if (quoted)
	{
		// Trailing backslashes now precede our closing quote, so double
		// them to keep that quote from being escaped.
		out.append(backslashes * 2, L'\\');
		out.push_back(L'"');
	}
	else
	{
		out.append(backslashes, L'\\');
	}
}

static std::wstring make_command_line_string(std::filesystem::path const& program, std::vector<std::string> const& args)
{
	// Basically the reverse of the rules described here:
	//  https://learn.microsoft.com/en-us/cpp/c-language/parsing-c-command-line-arguments?view=msvc-170

	std::wstring_view const program_string = program.native();

	// argv[0] has special CRT parsing rules. A valid Windows path cannot
	// contain '"', so quoting is required only when it contains space/tab.
	if (program_string.find(L'"') != std::wstring_view::npos)
	{
		throw std::invalid_argument{"program path contains a double quote"};
	}

	std::wstring result;

	if (contains_space_or_tab(program_string))
	{
		result.push_back(L'"');
		result.append(program_string);
		result.push_back(L'"');
	}
	else
	{
		result.append(program_string);
	}

	for (std::string const& arg : args)
	{
		result.push_back(L' ');
		append_argument(result, coverpp::windows::utf8_to_utf16le(arg));
	}

	return result;
}


namespace coverpp
{
int run_with_coverage(const CoverageParams& params)
{
    windows::WindowsCoverageSession coverage_session{params};

    auto reachable = coverage_session.collect_source_lines();
    if (params.verbosity >= 1)
    {
        std::println("Found {} reachable tracepoints",
                     coverpp::styled<ColorBold::cyan>(reachable.count_tracepoints()));
    }


    // Step #2: Find address of main function
    const auto main_entry_va = coverage_session.find_entrypoint();


    CoverageSink sink;


    // Step #3: Run the exe in a new process with coverage tracking

    // See this amazingly helpful resource: https://www.codeproject.com/Articles/43682/Writing-a-basic-Windows-debugger

	// Using a job object to make sure that the child process is killed when we exit
	auto job      = wil::unique_handle{THROW_LAST_ERROR_IF_NULL(CreateJobObjectW(nullptr, nullptr))};
	auto job_info = JOBOBJECT_EXTENDED_LIMIT_INFORMATION{
	    .BasicLimitInformation = {.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE},
	};
	THROW_IF_WIN32_BOOL_FALSE(
	    SetInformationJobObject(job.get(), JobObjectExtendedLimitInformation, &job_info, sizeof(job_info)));

    STARTUPINFOW si{};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi{};
    // Inherit env and workdir from the coverage process
    auto command_line = make_command_line_string(params.program, params.program_args);
    THROW_LAST_ERROR_IF_NOT(CreateProcessW(
        params.program.c_str(), command_line.data(),
        nullptr, nullptr, false, DEBUG_ONLY_THIS_PROCESS|CREATE_SUSPENDED, nullptr, nullptr,
        &si, &pi
    ));

	auto _ = Guard{[hProcess = pi.hProcess] { CloseHandle(hProcess); }};

	if (!AssignProcessToJobObject(job.get(), pi.hProcess))
	{
		TerminateProcess( pi.hProcess,1 );
		throw std::runtime_error("AssignProcessToJobObject failed");
	}

	ResumeThread(pi.hThread);
	CloseHandle(pi.hThread);

    const HANDLE hProcess = pi.hProcess;

    // Step #4: Set breakpoint in main function
    BreakpointDriver breakpoint_driver{hProcess};

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
        	coverage_session.dia().set_base_address(
                VirtualAddress{(std::uintptr_t) evt.u.CreateProcessInfo.lpBaseOfImage});

            // Set breakpoints at all reachable locations
            for (const auto& [source_file, dia_line_number] : coverage_session.enum_source_lines())
            {
                VirtualAddress va; // NOLINT(*-pro-type-member-init)
                THROW_IF_FAILED(dia_line_number.get_virtualAddress(&va.value));

                breakpoint_driver.set_breakpoint(va);

                if (params.verbosity >= 5)
                {
                    std::println("Set breakpoint at {}", va);
                }
            }

        	CloseHandle(evt.u.CreateProcessInfo.hFile); // We need to do this here as documented by Microsoft

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

            const VirtualAddress va{(std::uintptr_t) evt.u.Exception.ExceptionRecord.ExceptionAddress};

            const auto tracepoint = coverage_session.resolve_tracepoint(va);

            continue_status = DBG_EXCEPTION_NOT_HANDLED;

            if (evt.u.Exception.ExceptionRecord.ExceptionCode == STATUS_BREAKPOINT)
            {
				if (first_breakpoint)
				{
					first_breakpoint = false;
					continue_status  = DBG_CONTINUE;
					std::println("SUT ready (PID: {})", evt.dwProcessId);
					std::fflush(stdout);
					break;
				}
				else if (breakpoint_driver.has_breakpoint(va))
				{
					coverage_session.trace(sink, va);

					// Note: We could also set the Trap Flag here:  context.EFlags |= (1 << 8) | (1 << 16);
					//       That would make the CPU trap after every instruction with STATUS_SINGLE_STEP
					CONTEXT context{};
					context.ContextFlags = CONTEXT_CONTROL;
					THROW_LAST_ERROR_IF_NOT(GetThreadContext(hThread, &context));
					--context.Rip; // Decrement because we get here *after* the INT3 instruction executed
					THROW_LAST_ERROR_IF_NOT(SetThreadContext(hThread, &context));

					breakpoint_driver.remove_breakpoint(va);

					continue_status = DBG_CONTINUE;
				}
				else
				{
					// Not one of our breakpoints
					continue_status = DBG_EXCEPTION_NOT_HANDLED;
				}
            }

            if (continue_status == DBG_EXCEPTION_NOT_HANDLED)
            {
                const bool first_chance = evt.u.Exception.dwFirstChance;
                const auto& record = evt.u.Exception.ExceptionRecord;
                if (!first_chance || params.print_first_chance_seh_exceptions)
                {
                    std::println(
                        "{} {} encountered at {}",
                        first_chance ? "First-chance" : "Unhandled",
                        describe_seh_exception(record.ExceptionCode, record.ExceptionInformation),
                        tracepoint ? std::format("{}:{}", tracepoint->source_file.u8string(), tracepoint->tracepoint.lineBegin)
                                   : std::format("{}", record.ExceptionAddress)
                    );
                }
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
                     coverpp::styled<ColorBold::cyan>(sink.count_tracepoints()));
    }

    BasicReport report = process_coverage_sink(sink);
    BasicReport reachable_report = process_coverage_sink(reachable);

    RawExporter raw_exporter;
    raw_exporter.run(report, reachable_report, params);

    return *exit_code;
}
}
