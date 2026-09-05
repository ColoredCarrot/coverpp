#pragma once

#include <filesystem>
#include <format>
#include <unordered_map>
#include <set>
#include <ranges>

namespace coverpp
{
struct FileTracepoint
{
    /** 1-based; zero means unknown. */
    unsigned lineBegin;
    unsigned columnBegin;
    unsigned lineEnd;
    unsigned columnEnd;

    bool operator==(const FileTracepoint&) const = default;
    std::strong_ordering operator<=>(const FileTracepoint&) const = default;
};

struct Tracepoint
{
    std::filesystem::path source_file;
    FileTracepoint tracepoint;
};

class CoverageSink
{
public:
    void track_coverage(const Tracepoint& tracepoint);

    const std::unordered_map<std::filesystem::path, std::set<FileTracepoint>>& tracepoints() const;

    std::size_t count_tracepoints() const;

private:
    // Note: No duplicate tracepoints are recorded
    std::unordered_map<std::filesystem::path, std::set<FileTracepoint>> m_tracepoints;

    friend struct std::formatter<CoverageSink>;
};
}

template<>
struct std::formatter<coverpp::FileTracepoint>
{
    constexpr auto parse(std::format_parse_context& ctx)
    {
        return ctx.begin();
    }

    auto format(const coverpp::FileTracepoint& tracepoint, std::format_context& ctx) const
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
