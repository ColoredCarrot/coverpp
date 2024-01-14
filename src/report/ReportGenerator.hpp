#pragma once

#include "Report.hpp"

namespace coverpp
{
class ReportGenerator
{
public:
    virtual ~ReportGenerator() = default;

    virtual void generate_report(const Report& report, const Report& reachability_report) = 0;
};
}
