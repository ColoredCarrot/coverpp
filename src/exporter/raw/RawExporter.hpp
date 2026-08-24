#pragma once

#include "../../CoverageParams.hpp"
#include "../../report/BasicReport.hpp"

namespace coverpp
{
class RawExporter
{
public:
    void run(const BasicReport& covered, const BasicReport& reachable, CoverageParams const& params);

};
}
