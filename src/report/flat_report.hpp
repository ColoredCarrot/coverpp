#pragma once

#include <coverage_report_generated.h>

#include <filesystem>
#include <generator>
#include <unordered_set>

namespace coverpp
{
struct FlatReportEntry
{
	std::filesystem::path        source_file;
	std::unordered_set<unsigned> reachable_lines;
	std::unordered_set<unsigned> covered_lines;
};

std::generator<FlatReportEntry> flatten_report(Coverpp::Report::CoverageReportT const& report);
} // namespace coverpp
