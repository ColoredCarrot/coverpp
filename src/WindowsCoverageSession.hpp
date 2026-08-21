#pragma once

#include "CoverageEngine.hpp"
#include "DiaAccessor.hpp"
#include "types.hpp"

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

    VirtualAddress find_entrypoint();

    std::optional<std::filesystem::path> trace(CoverageSink& sink, VirtualAddress va);

    std::optional<std::pair<std::filesystem::path, Tracepoint>> resolve_tracepoint(VirtualAddress va);

    DiaAccessor& dia();

private:
    CoverageParams m_params;

    DiaAccessor m_dia;

	[[nodiscard]] bool should_trace(std::filesystem::path const& source_file) const;
};
}
