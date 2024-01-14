#include "Report.hpp"

namespace coverpp
{
std::unordered_set<unsigned>& FileReport::covered_lines()
{
    return m_covered_lines;
}
const std::unordered_set<unsigned>& FileReport::covered_lines() const
{
    return m_covered_lines;
}

std::unordered_map<std::filesystem::path, FileReport>& Report::file_reports()
{
    return m_file_reports;
}
const std::unordered_map<std::filesystem::path, FileReport>& Report::file_reports() const
{
    return m_file_reports;
}
}
