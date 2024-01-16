#pragma once

#include "Exporter.hpp"

namespace coverpp
{
class SourceFileExporter : public Exporter
{
public:
    explicit SourceFileExporter(std::filesystem::path output_directory);

    void run(const BasicReport& report, const BasicReport& reachability_report) override;

private:
    std::filesystem::path m_dir;
};
}
