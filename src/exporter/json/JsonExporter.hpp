#pragma once

#include "../Exporter.hpp"
#include "json_export_options.hpp"

namespace coverpp
{
class JsonExporter : public Exporter
{
public:
	void run(Coverpp::Report::CoverageReportT const& report) override;

	static inline JsonExportOptions options;
};
} // namespace coverpp
