#include "CoverageSink.hpp"

namespace coverpp
{
void CoverageSink::track_coverage(const std::filesystem::path& source_file, const Tracepoint& tracepoint)
{
    m_tracepoints[source_file].push_back(tracepoint);
}
}

