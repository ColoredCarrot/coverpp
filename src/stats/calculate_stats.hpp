#pragma once

#include <coverage_report_generated.h>

namespace coverpp
{
void calculate_stats(Coverpp::Report::RootT& root);
void calculate_stats(Coverpp::Report::CoverageReportT& report);
} // namespace coverpp
