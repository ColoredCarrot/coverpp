#pragma once

#include "../Exporter.hpp"

namespace coverpp
{
class TeamCityStatisticsExporter:public Exporter
{
public:
    void run(Coverpp::Report::CoverageReportT const& report) override;
};
}
