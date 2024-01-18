#pragma once

#include "CoverageEngine.hpp"
#include "DiaAccessor.hpp"
#include "types.hpp"
#include "polyfill/generator.hpp"

namespace coverpp::windows
{
class WindowsCoverageSession : public CoverageSession
{
public:
    explicit WindowsCoverageSession(CoverageParams params);

    std::generator<std::pair<const std::filesystem::path&, IDiaLineNumber&>> enum_source_lines();

    CoverageSink collect_source_lines() override;

    VirtualAddress find_entrypoint();

    std::optional<std::filesystem::path> trace(CoverageSink& sink, VirtualAddress va);

    std::optional<std::pair<std::filesystem::path, Tracepoint>> resolve_tracepoint(VirtualAddress va);

    DiaAccessor& dia();

private:
    CoverageParams m_params;

    DiaAccessor m_dia;
};
}
