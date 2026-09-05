#include "merge_reports.hpp"

#include "../report/report_file_utils.hpp"
#include "../stats/calculate_stats.hpp"
#include "../util/progress_bar.hpp"

#include <flatbuffers/flatbuffers.h>

#include <coverage_report_generated.h>

#include <fstream>
#include <print>
#include <ranges>

namespace coverpp
{
static bool should_merge_children(Coverpp::Report::PathReportUnion const& a, Coverpp::Report::PathReportUnion const& b)
{
	if (auto* a_file = a.AsFileReport())
	{
		auto* b_file = b.AsFileReport();
		return b_file && a_file->path == b_file->path;
	}

	auto* a_dir = a.AsDirectoryReport();
	auto* b_dir = b.AsDirectoryReport();
	return b_dir && a_dir->name == b_dir->name;
}

static void merge_children(std::vector<Coverpp::Report::PathReportUnion>&       accumulated,
                           std::vector<Coverpp::Report::PathReportUnion> const& next);

template<typename T>
static void merge_sets(std::vector<T>& accumulated, std::vector<T> const& next)
{
	auto result = std::vector<T>();
	result.reserve(accumulated.size() + next.size());
	std::ranges::set_union(accumulated, next, std::back_inserter(result));
	accumulated = std::move(result);
}

static void merge(Coverpp::Report::FileReportT& accumulated, Coverpp::Report::FileReportT const& next)
{
	merge_sets(accumulated.reachable_lines, next.reachable_lines);
	merge_sets(accumulated.covered_lines, next.covered_lines);
}
static void merge(Coverpp::Report::DirectoryReportT& accumulated, Coverpp::Report::DirectoryReportT const& next)
{
	merge_children(accumulated.children, next.children);
}
static void merge(Coverpp::Report::PathReportUnion& accumulated, Coverpp::Report::PathReportUnion const& next)
{
	if (auto* a_file = accumulated.AsFileReport())
	{
		merge(*a_file, *next.AsFileReport());
	}
	else
	{
		merge(*accumulated.AsDirectoryReport(), *next.AsDirectoryReport());
	}
}

static void merge_children(std::vector<Coverpp::Report::PathReportUnion>&       accumulated,
                           std::vector<Coverpp::Report::PathReportUnion> const& next)
{
	for (auto& next_child : next)
	{
		auto const acc_child_it = std::ranges::find_if(
		    accumulated, [&](auto& acc_child) { return should_merge_children(acc_child, next_child); });
		if (acc_child_it != accumulated.end())
		{
			merge(*acc_child_it, next_child);
		}
		else
		{
			accumulated.push_back(next_child);
		}
	}
}

static bool should_merge_roots(Coverpp::Report::RootT const& a, Coverpp::Report::RootT const& b)
{
	return a.path == b.path && a.directory_separator == b.directory_separator;
}

static void merge_roots(Coverpp::Report::RootT& accumulated, Coverpp::Report::RootT const& next)
{
	merge_children(accumulated.children, next.children);
}

static void merge_reports(Coverpp::Report::CoverageReportT& accumulated, Coverpp::Report::CoverageReportT const& next)
{
	for (auto& next_root : next.roots)
	{
		auto const acc_root_it = std::ranges::find_if(
		    accumulated.roots, [&](auto& acc_root) { return should_merge_roots(next_root, acc_root); });
		if (acc_root_it != accumulated.roots.end())
		{
			merge_roots(*acc_root_it, next_root);
		}
		else
		{
			accumulated.roots.push_back(next_root);
		}
	}
}

void merge_reports(MergeOptions const& options)
{
	Coverpp::Report::CoverageReportT accumulated{};

	accumulated.coverpp_version = COVERPP_VERSION_STRING;

	auto progress = ProgressBar{options.input_files.size()};
	for (auto const& [i, input_file] : options.input_files | std::views::enumerate)
	{
		auto const& report    = read_report(input_file);

		accumulated.timestamp = report.timestamp;
		accumulated.coverpp_version = report.coverpp_version;

		merge_reports(accumulated, report);

		progress.update(i + 1);
	}

	calculate_stats(accumulated);

	write_report(options.output_file, accumulated);

	progress.finish();
	std::println("Done");
}
} // namespace coverpp
