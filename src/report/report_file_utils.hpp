#pragma once

#include <coverage_report_generated.h>

#include <filesystem>

namespace coverpp
{
Coverpp::Report::CoverageReportT read_report(std::filesystem::path const& file);

void write_report(std::filesystem::path const& file, Coverpp::Report::CoverageReportT const& report);

} // namespace coverpp
