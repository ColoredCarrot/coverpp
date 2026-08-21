#include "src/merge/merge_reports.hpp"
#include "src/run.hpp"
#include "src/serve/serve.hpp"
#include "src/util/console_color.hpp"

#include <CLI11.hpp>
#include <print>
#include <iostream>
#include <filesystem>
#include <cassert>
#include <unordered_map>

#define NOMINMAX

#include "src/exporter/clion/CLionExporter.hpp"
#include "src/exporter/html/HtmlExporter.hpp"


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

int main(int argc, char** argv)
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

    CLI::App app{"Cover++"};
    app.failure_message(CLI::FailureMessage::help);

    const auto run_app = app.add_subcommand("run");

    coverpp::CoverageParams params{};
    run_app->add_option("-s,--source", params.source_dir, "Source directory")->check(
        CLI::ExistingDirectory)->required();
    run_app->add_option("-p,--program", params.program, "Executable")->check(CLI::ExistingFile)->required();
    run_app->add_option("-a,--program-args", params.program_args, "Arguments to pass to the executable");
    run_app->add_option("-d,--debug-info", params.debug_info, "PDB file")->check(CLI::ExistingFile);
    run_app->add_option("-o,--out-dir", params.out_dir, "Output directory")->default_val("./coverpp-report");
    run_app->add_flag("-v,--verbose", params.verbosity, "Print more messages to the console");
    run_app->add_flag("--print-first-chance-seh", params.print_first_chance_seh_exceptions,
                      "Print first-chance SEH exceptions to the console");

    coverpp::ServeOptions serve_options{.report_path{"./coverpp-report/report.coverpp"}};
    const auto serve_app = app.add_subcommand("view", "View coverage results in your browser");
    serve_app->alias("serve");
    serve_app->add_option("-p,--port", serve_options.port)->default_val(8080)->check(CLI::NonNegativeNumber);
    serve_app->add_option("-d,--data", serve_options.report_path)->check(CLI::ExistingFile);
    serve_app->add_option("--coverpp-install-dir", serve_options.coverpp_install_dir)
        ->check(CLI::ExistingDirectory)
        ->required(argc == 0)
        ->default_val(
            argc == 0 ? std::string{""} : std::filesystem::weakly_canonical(argv[0]).parent_path().u8string());

	auto       merge_options = coverpp::MergeOptions{};
	auto const merge_app     = app.add_subcommand("merge", "Merge multiple reports into one");
	merge_app->add_option("-o,--output", merge_options.output_file);
	merge_app->add_option("input-files", merge_options.input_files)->expected(-1);

	auto const export_app      = app.add_subcommand("export", "Export coverage results");
	auto const export_html_app = export_app->add_subcommand("html");
	export_html_app->add_option("input", coverpp::HtmlExporter::options.report_file)->check(CLI::ExistingFile);
	export_html_app->add_option("-o,--out-dir", coverpp::HtmlExporter::options.out_dir);
	auto const export_clion_app = export_app->add_subcommand("clion");
	export_clion_app->add_option("input", coverpp::CLionExporter::options.report_file)->check(CLI::ExistingFile);
	export_clion_app->add_option("-o,--out-dir", coverpp::CLionExporter::options.out_dir);

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

	if (*merge_app)
	{
		coverpp::merge_reports(merge_options);
		return 0;
	}

	if (*export_html_app)
	{
		return coverpp::run_exporter<coverpp::HtmlExporter>();
	}

	if (*export_clion_app)
	{
		return coverpp::run_exporter<coverpp::CLionExporter>();
	}

	if (params.debug_info.empty())
	{
		params.debug_info = params.program.parent_path() / (params.program.stem().string() + ".pdb");
	}
	params.program    = canonical(params.program);
	params.debug_info = canonical(params.debug_info);

    if (params.verbosity > 0)
    {
        std::println("Cover++");
        std::println("==========================");
        std::println("Source directory: {}\nExecutable:       {}\nDebug info:       {}\nOutput directory: {}",
                     params.source_dir.u8string(),
                     params.program.u8string(),
                     params.debug_info.u8string(),
                     absolute(params.out_dir).u8string());
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
