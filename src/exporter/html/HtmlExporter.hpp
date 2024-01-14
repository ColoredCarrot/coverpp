#pragma once

#include "../../report/BasicReport.hpp"

namespace coverpp
{
class HtmlExporter
{
public:
    explicit HtmlExporter(std::filesystem::path output_directory);

    void export_report(const BasicReport& covered, const BasicReport& reachable);

private:
    std::filesystem::path m_dir;
};
}
