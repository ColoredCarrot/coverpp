#pragma once

#include "../run/CoverageSink.hpp"
#include "BasicReport.hpp"

namespace coverpp
{
BasicReport process_coverage_sink(CoverageSink& sink);
}
