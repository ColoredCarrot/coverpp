#pragma once

#include "../report/BasicReport.hpp"

namespace coverpp
{
class Exporter
{
public:
    virtual ~Exporter() = default;

    virtual void run(const BasicReport& covered, const BasicReport& reachable) = 0;
};
}
