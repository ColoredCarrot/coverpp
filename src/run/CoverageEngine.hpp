#pragma once

#include "CoverageSink.hpp"
#include "CoverageParams.hpp"

namespace coverpp
{
class CoverageSession
{
public:
    virtual ~CoverageSession() = default;

    virtual CoverageSink collect_source_lines() = 0;
};

class CoverageEngine
{
public:
    virtual ~CoverageEngine() = default;

    virtual std::unique_ptr<CoverageSession> create_session(const CoverageParams& params) = 0;
};
}
