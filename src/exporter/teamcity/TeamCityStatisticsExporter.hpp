#pragma once

#include "../Exporter.hpp"

namespace coverpp
{
class TeamCityStatisticsExporter:public Exporter
{
public:
    void run(const BasicReport& covered, const BasicReport& reachable) override;
};
}
