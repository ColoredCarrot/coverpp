#include "src/merge/merge_reports.hpp"
#include "src/run.hpp"
#include "src/serve/serve.hpp"
#include "src/util/console_color.hpp"

#include <CLI11.hpp>
#include <print>
#include <iostream>
#include <filesystem>
#include <cassert>
#include <ranges>

#define NOMINMAX

#include "src/docs/docs.hpp"
#include "src/util/encodings_util.hpp"
#include "src/exporter/clion/CLionExporter.hpp"
#include "src/exporter/html/HtmlExporter.hpp"
#include "src/exporter/json/JsonExporter.hpp"
#include "src/remap/remap.hpp"


#include <wil/com.h>
#include <dia2.h>
#include <psapi.h>
#include <intrin.h>


#define THROW_LAST_ERROR_IF_NOT(x) THROW_LAST_ERROR_IF(!(x))

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

int wmain(int argc, wchar_t** argv_unicode)
try
{
#ifdef _WIN32
    // Tell the console to interpret outputted bytes as UTF-8
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    // Enable buffering to prevent VS from chopping up UTF-8 byte sequences
    setvbuf(stdout, nullptr, _IOFBF, 1000);
#endif

    std::atexit([] { std::cout << coverpp::Style::reset; });

	// Even though we compile with /utf-8, `char** argv` would still be ANSI.
	// So, we use `wmain` and convert it to UTF-8.
	auto args = std::span{argv_unicode, static_cast<std::size_t>(argc)}
	            | std::views::transform([](wchar_t const* w) { return coverpp::windows::utf16le_to_utf8(w); })
	            | std::ranges::to<std::vector>();
	auto argv_data
	    = args | std::views::transform([](std::string const& s) { return s.c_str(); }) | std::ranges::to<std::vector>();
	auto argv = argv_data.data();

	// Treat a simple, no-other-args call with `--help` the same as calling the `help` command.
	// In all other cases, use the default handling for `--help`.
	if (argc == 2 && args[1] == "--help")
	{
		args[1] = "help";
	}

    CLI::App app{"Cover++"};
    app.failure_message(CLI::FailureMessage::help);

    const auto run_app = app.add_subcommand("run");

    coverpp::CoverageParams params{};
    run_app->add_option("-s,--source", params.source_dir, "Source directory");
    run_app->add_option("-d,--debug-info", params.debug_info, "PDB file")->check(CLI::ExistingFile);
    run_app->add_option("-o,--out", params.out_file, "Output file")->default_val("./report.coverpp");
	run_app->add_option("--exclude-source-files-regex", params.exclude_source_files_regex, "Regex for source files to exclude (matches on generic paths with /)");
    run_app->add_flag("-v,--verbose", params.verbosity, "Print more messages to the console");
    run_app->add_flag("--print-first-chance-seh", params.print_first_chance_seh_exceptions,
                      "Print first-chance SEH exceptions to the console");
	run_app->prefix_command(CLI::PrefixCommandMode::PositionalOnly);
	run_app->usage([&] { return std::format("{} run [OPTIONS] <program> [args...]", app.get_name()); });

    coverpp::ServeOptions serve_options{.report_path{"./report.coverpp"}, .open = true};
    const auto serve_app = app.add_subcommand("view", "View coverage results in your browser");
    serve_app->alias("serve");
    serve_app->add_option("report", serve_options.report_path)->check(CLI::ExistingFile);
    serve_app->add_option("-p,--port", serve_options.port)->default_val(8080)->check(CLI::NonNegativeNumber);
    serve_app->add_option("--coverpp-install-dir", serve_options.coverpp_install_dir)
        ->check(CLI::ExistingDirectory)
        ->required(argc == 0)
        ->default_val(
            argc == 0 ? std::string{""} : std::filesystem::weakly_canonical(argv[0]).parent_path().u8string());
	serve_app->add_flag("--open,!--no-open", serve_options.open)->default_val(true);

    coverpp::DocsOptions docs_options{.open = true};
    const auto docs_app = app.add_subcommand("docs", "Open the documentation in your browser");
	docs_app->alias("help");
    docs_app->add_option("-p,--port", docs_options.port)->default_val(8080)->check(CLI::NonNegativeNumber);
    docs_app->add_option("--coverpp-install-dir", docs_options.coverpp_install_dir)
        ->check(CLI::ExistingDirectory)
        ->required(argc == 0)
        ->default_val(
            argc == 0 ? std::string{""} : std::filesystem::weakly_canonical(argv[0]).parent_path().u8string());
	docs_app->add_flag("--open,!--no-open", docs_options.open)->default_val(true);

	auto       merge_options = coverpp::MergeOptions{};
	auto const merge_app     = app.add_subcommand("merge", "Merge multiple reports into one");
	merge_app->add_option("-o,--output", merge_options.output_file)->required();
	merge_app->add_option("input-files", merge_options.input_files)->expected(-1);

	auto       remap_options = coverpp::RemapOptions{};
	auto const remap_app     = app.add_subcommand("remap", "Remap source roots");
	remap_app->add_option("report", remap_options.report)->check(CLI::ExistingFile);
	remap_app->add_option("--from", remap_options.from);
	remap_app->add_option("--to", remap_options.to)->required();

	auto const export_app      = app.add_subcommand("export", "Export coverage results");
	auto const export_html_app = export_app->add_subcommand("html");
	export_html_app->add_option("input", coverpp::HtmlExporter::options.report_file)->check(CLI::ExistingFile);
	export_html_app->add_option("-o,--out-dir", coverpp::HtmlExporter::options.out_dir);
	auto const export_clion_app = export_app->add_subcommand("clion");
	export_clion_app->add_option("input", coverpp::CLionExporter::options.report_file)->check(CLI::ExistingFile);
	export_clion_app->add_option("-o,--out-dir", coverpp::CLionExporter::options.out_dir);
	auto const export_json_app = export_app->add_subcommand("json");
	export_json_app->add_option("input", coverpp::JsonExporter::options.report_file)->check(CLI::ExistingFile);
	export_json_app->add_option("-o,--out", coverpp::JsonExporter::options.out_file)->default_val("-");

    try
    {
        app.parse(argc, argv);
    }
    catch (const CLI::ParseError& e)
    {
        std::cout << (e.get_exit_code() == 0 ? coverpp::Color::blue : coverpp::Color::red);
        return app.exit(e);
    };

    if (*serve_app)
    {
        return coverpp::serve(serve_options);
    }

    if (*docs_app)
    {
        return coverpp::serve_docs(docs_options);
    }

	if (*merge_app)
	{
		coverpp::merge_reports(merge_options);
		return 0;
	}

	if (*remap_app)
	{
		return coverpp::remap(remap_options);
	}

	if (*export_html_app)
	{
		return coverpp::run_exporter<coverpp::HtmlExporter>();
	}

	if (*export_clion_app)
	{
		return coverpp::run_exporter<coverpp::CLionExporter>();
	}

	if (*export_json_app)
	{
		return coverpp::run_exporter<coverpp::JsonExporter>();
	}

	if (!*run_app)
	{
		throw std::logic_error{"No command specified"};
	}

	auto program_invocation = run_app->remaining();
	if (program_invocation.empty())
	{
		std::cout << coverpp::Color::red << "No program specified" << std::endl;
		return 1;
	}

	params.program      = std::filesystem::canonical(program_invocation[0]);
	params.program_args = std::vector(program_invocation.begin() + 1, program_invocation.end());

	if (params.debug_info.empty())
	{
		params.debug_info = params.program.parent_path() / (params.program.stem().string() + ".pdb");
	}
	params.debug_info = canonical(params.debug_info);

    if (params.verbosity > 0)
    {
        std::println("Cover++");
        std::println("==========================");
        std::println("Source directory: {}\nExecutable:       {}\nDebug info:       {}\nOutput directory: {}",
                     params.source_dir.u8string(),
                     params.program.u8string(),
                     params.debug_info.u8string(),
                     absolute(params.out_file).u8string());
        std::println("");
    }

    try
    {
        CoInitializeGuard guard;

        return coverpp::run_with_coverage(params);
    }
    catch (const wil::ResultException& ex)
    {
        std::println(std::cerr, "Windows Exception: {}", ex.what());
        return 1;
    }
}
catch (const std::exception& ex)
{
    std::println(std::cerr, "Error: {}", ex.what());
}
catch (...)
{
    std::println(std::cerr, "Unknown error");
}
