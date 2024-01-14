#pragma once

#include <filesystem>
#include <unordered_map>

namespace coverpp
{
struct Tracepoint
{
    /** 1-based; zero means unknown. */
    unsigned lineBegin, lineEnd;
    unsigned columnBegin, columnEnd;
};

class CoverageSink
{
public:
    void track_coverage(const std::filesystem::path& source_file, const Tracepoint& tracepoint);

private:
    std::unordered_map<std::filesystem::path, std::vector<Tracepoint>> m_tracepoints;
};
}
