#pragma once

#include "ReportGenerator.hpp"

namespace coverpp
{
class SourceFileReportGenerator : public ReportGenerator
{
public:
    explicit SourceFileReportGenerator(std::filesystem::path output_directory);

    void generate_report(const BasicReport& report, const BasicReport& reachability_report) override;

private:
    std::filesystem::path m_dir;
};
}
