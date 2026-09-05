#pragma once

#include "CoverageParams.hpp"
#include "report/BasicReport.hpp"

namespace coverpp
{
void create_report(BasicReport const& covered, BasicReport const& reachable, CoverageParams const& params);
}
