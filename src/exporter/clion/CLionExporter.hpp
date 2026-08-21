#pragma once

#include "../Exporter.hpp"
#include "clion_export_options.hpp"

namespace coverpp
{
class CLionExporter : public Exporter
{
public:
	void run(Coverpp::Report::CoverageReportT const& report) override;

	static inline CLionExportOptions options;
};
} // namespace coverpp
