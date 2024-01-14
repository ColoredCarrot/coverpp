#pragma once

#include "BasicReport.hpp"

namespace coverpp
{
class ReportGenerator
{
public:
    virtual ~ReportGenerator() = default;

    virtual void generate_report(const BasicReport& report, const BasicReport& reachability_report) = 0;
};
}
