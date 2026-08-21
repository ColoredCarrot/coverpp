#pragma once

#include "export_options.hpp"
#include "../report/report_file_utils.hpp"

#include <coverage_report_generated.h>

#include <print>

namespace coverpp
{
class Exporter
{
public:
	virtual ~Exporter() = default;

	virtual void run(Coverpp::Report::CoverageReportT const& report) = 0;
};

template<std::derived_from<Exporter> TExporter>
int run_exporter(ExportOptions const& options = TExporter::options)
{
	TExporter{}.run(read_report(options.report_file));
	std::println("Export finished.");
	return 0;
}
} // namespace coverpp
