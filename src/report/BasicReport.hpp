#pragma once

#include <unordered_map>
#include <unordered_set>
#include <filesystem>

namespace coverpp
{
class BasicFileReport
{
public:
    std::unordered_set<unsigned>& covered_lines();
    const std::unordered_set<unsigned>& covered_lines() const;

private:
    std::unordered_set<unsigned> m_covered_lines;
};

class BasicReport
{
public:
    std::unordered_map<std::filesystem::path, BasicFileReport>& file_reports();
    const std::unordered_map<std::filesystem::path, BasicFileReport>& file_reports() const;

private:
    std::unordered_map<std::filesystem::path, BasicFileReport> m_file_reports;
};
}
