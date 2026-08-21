#pragma once

#include "../Exporter.hpp"
#include "html_export_options.hpp"

namespace coverpp
{
class HtmlExporter : public Exporter
{
public:
    void run(Coverpp::Report::CoverageReportT const& report) override;

    static inline HtmlExportOptions options;
};
}
