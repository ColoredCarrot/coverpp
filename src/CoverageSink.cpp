#include "CoverageSink.hpp"

namespace coverpp
{
void CoverageSink::track_coverage(const std::filesystem::path& source_file, const Tracepoint& tracepoint)
{
    m_tracepoints[source_file].emplace(tracepoint);
}
}

std::format_context::iterator
std::formatter<coverpp::CoverageSink>::format(const coverpp::CoverageSink& sink, std::format_context& ctx) const
{
    auto out = ctx.out();
    for (const auto& [source_file, tracepoints]: sink.m_tracepoints)
    {
        out = std::format_to(
            out,
            "{} @ {}\n",
            source_file.string(),
            tracepoints
            | std::views::transform([](const auto& tp) { return std::format("{}", tp); })
            | std::views::join_with(std::string_view(", "))
            | std::ranges::to<std::string>()
        );
    }
    return out;
}
