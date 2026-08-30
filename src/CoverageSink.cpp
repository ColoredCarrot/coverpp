#include "CoverageSink.hpp"

namespace coverpp
{
void CoverageSink::track_coverage(const Tracepoint& tracepoint)
{
    m_tracepoints[tracepoint.source_file].emplace(tracepoint.tracepoint);
}

const std::unordered_map<std::filesystem::path, std::set<FileTracepoint>>& CoverageSink::tracepoints() const
{
    return m_tracepoints;
}

std::size_t CoverageSink::count_tracepoints() const
{
    return std::ranges::fold_left(
        std::views::values(m_tracepoints) | std::views::transform(std::ranges::size),
        0,
        std::plus{}
    );
}
}

std::format_context::iterator
std::formatter<coverpp::CoverageSink>::format(const coverpp::CoverageSink& sink, std::format_context& ctx) const
{
    auto out = ctx.out();
    for (const auto& [source_file, tracepoints] : sink.m_tracepoints)
    {
        out = std::format_to(
            out,
            "{} @ {}\n",
            source_file.u8string(),
            tracepoints
            | std::views::transform([](const auto& tp) { return std::format("{}", tp); })
            | std::views::join_with(std::string_view(", "))
            | std::ranges::to<std::string>()
        );
    }
    return out;
}
