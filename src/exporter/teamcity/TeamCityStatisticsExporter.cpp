#include "TeamCityStatisticsExporter.hpp"

#include <filesystem>
#include <iostream>

namespace coverpp
{
void TeamCityStatisticsExporter::run(Coverpp::Report::CoverageReportT const& report)
{
    //TODO
    std::filesystem::path file_path;
    std::println(
        std::clog,
        "##teamcity[importData type='dotNetCoverage' tool='Cover++' path='{}']",
        file_path.u8string()
    );
}
}
