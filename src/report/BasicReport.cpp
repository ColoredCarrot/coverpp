#include "BasicReport.hpp"

namespace coverpp
{
std::set<unsigned>& BasicFileReport::covered_lines()
{
    return m_covered_lines;
}
const std::set<unsigned>& BasicFileReport::covered_lines() const
{
    return m_covered_lines;
}

std::unordered_map<std::filesystem::path, BasicFileReport>& BasicReport::file_reports()
{
    return m_file_reports;
}
const std::unordered_map<std::filesystem::path, BasicFileReport>& BasicReport::file_reports() const
{
    return m_file_reports;
}
}
