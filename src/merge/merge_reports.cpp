#include "merge_reports.hpp"

#include "../stats/calculate_stats.hpp"

#include <flatbuffers/flatbuffers.h>

#include <coverage_report_generated.h>

#include <fstream>

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

static Coverpp::Report::CoverageReportT read(std::filesystem::path const& file)
{
	std::ifstream stream{file, std::ios::binary | std::ios::in};

	stream.seekg(0, std::ios::end);
	auto const length = stream.tellg();
	stream.seekg(0, std::ios::beg);

	auto data = std::vector<char>(length);
	stream.read(data.data(), length);
	stream.close();

	Coverpp::Report::CoverageReportT report;
	Coverpp::Report::GetCoverageReport(data.data())->UnPackTo(&report);

	return report;
}

static void write(std::filesystem::path const& file, Coverpp::Report::CoverageReportT const& report)
{
	flatbuffers::FlatBufferBuilder builder{1024};
	builder.Finish(Coverpp::Report::CoverageReport::Pack(builder, &report));

	std::filesystem::create_directories(file.parent_path());
	std::ofstream stream{file, std::ios_base::out | std::ios_base::trunc | std::ios_base::binary};
	stream.exceptions(std::ios_base::badbit | std::ios_base::failbit);
	stream.write(reinterpret_cast<char const*>(builder.GetBufferPointer()), builder.GetSize());
}

void merge_reports(MergeOptions const& options)
{
	Coverpp::Report::CoverageReportT accumulated{};

	for (auto const& input_file : options.input_files)
	{
		merge_reports(accumulated, read(input_file));
	}

	calculate_stats(accumulated);

	write(options.output_file, accumulated);
}
} // namespace coverpp
