#include <cassert>
#include "CoverageProcessor.hpp"

namespace coverpp
{
Report process_coverage_sink(CoverageSink& sink)
{
    Report report;

    for (const auto& [source_file, tracepoints]: sink.tracepoints())
    {
        auto& covered_lines = report.file_reports()[source_file].covered_lines();
        for (const auto& tracepoint: tracepoints)
        {
            assert(tracepoint.lineBegin && tracepoint.lineEnd);//TODO might be zero if no information (when?)

            covered_lines.insert_range(std::views::iota(tracepoint.lineBegin, tracepoint.lineEnd + 1));
        }
    }

    return report;
}
}
