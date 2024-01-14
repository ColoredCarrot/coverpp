#pragma once

#include "CoverageEngine.hpp"
#include "DiaAccessor.hpp"
#include "types.hpp"

namespace coverpp::windows
{
class WindowsCoverageSession : public CoverageSession
{
public:
    explicit WindowsCoverageSession(CoverageParams params);

    CoverageSink collect_source_lines() override;

    VirtualAddress find_entrypoint();

    bool trace(CoverageSink& sink, VirtualAddress va);

private:
    CoverageParams m_params;

    DiaAccessor m_dia;
};
}
