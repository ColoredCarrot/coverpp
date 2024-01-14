#pragma once

#include "../../CoverageSink.hpp"

namespace coverpp
{
class HtmlExporter
{
public:
    explicit HtmlExporter(std::filesystem::path output_directory);

    void export_report(const CoverageSink& covered, const CoverageSink& reachable);

private:
    std::filesystem::path m_dir;
};
}
