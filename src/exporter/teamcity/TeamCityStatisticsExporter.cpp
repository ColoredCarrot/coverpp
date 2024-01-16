#include "TeamCityStatisticsExporter.hpp"

#include <iostream>

namespace coverpp
{
void TeamCityStatisticsExporter::run(const BasicReport& covered, const BasicReport& reachable)
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
