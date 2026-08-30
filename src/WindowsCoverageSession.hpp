#pragma once

#include "CoverageEngine.hpp"
#include "DiaAccessor.hpp"
#include "types.hpp"

#include <optional>

#if defined(__cpp_lib_generator)
#include <generator>
#else
#include "polyfill/generator.hpp"
#endif

namespace coverpp::windows
{
class WindowsCoverageSession : public CoverageSession
{
public:
    explicit WindowsCoverageSession(CoverageParams params);

    std::generator<std::pair<const std::filesystem::path&, IDiaLineNumber&>> enum_source_lines();

    CoverageSink collect_source_lines() override;

    std::optional<VirtualAddress> find_entrypoint();

    void trace(CoverageSink& sink, VirtualAddress va);

    std::optional<Tracepoint> resolve_tracepoint(VirtualAddress va);

    DiaAccessor& dia();

private:
    CoverageParams m_params;

    DiaAccessor m_dia;

	[[nodiscard]] bool should_trace(std::filesystem::path const& source_file) const;
};
}
