#pragma once

#include <filesystem>
#include <format>
#include <unordered_map>
#include <set>
#include <ranges>

namespace coverpp
{
struct Tracepoint
{
    /** 1-based; zero means unknown. */
    unsigned lineBegin, lineEnd;
    unsigned columnBegin, columnEnd;

    bool operator==(const Tracepoint&) const = default;
    std::strong_ordering operator<=>(const Tracepoint&) const = default;
};

class CoverageSink
{
public:
    void track_coverage(const std::filesystem::path& source_file, const Tracepoint& tracepoint);

    const std::unordered_map<std::filesystem::path, std::set<Tracepoint>>& tracepoints() const;

private:
    // Note: No duplicate tracepoints are recorded
    std::unordered_map<std::filesystem::path, std::set<Tracepoint>> m_tracepoints;

    friend struct std::formatter<CoverageSink>;
};
}

template<>
struct std::formatter<coverpp::Tracepoint>
{
    constexpr auto parse(std::format_parse_context& ctx)
    {
        return ctx.begin();
    }

    auto format(const coverpp::Tracepoint& tracepoint, std::format_context& ctx) const
    {
        return std::format_to(ctx.out(), "{}:{} - {}:{}",
                              tracepoint.lineBegin, tracepoint.columnBegin,
                              tracepoint.lineEnd, tracepoint.columnEnd);
    }
};

template<>
struct std::formatter<coverpp::CoverageSink>
{
    constexpr auto parse(std::format_parse_context& ctx)
    {
        return ctx.begin();
    }

    std::format_context::iterator format(const coverpp::CoverageSink& sink, std::format_context& ctx) const;
};
