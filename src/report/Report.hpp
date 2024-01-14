#pragma once

#include <unordered_map>
#include <unordered_set>
#include <filesystem>

namespace coverpp
{
class FileReport
{
public:
    std::unordered_set<unsigned>& covered_lines();
    const std::unordered_set<unsigned>& covered_lines() const;

private:
    std::unordered_set<unsigned> m_covered_lines;
};

class Report
{
public:
    std::unordered_map<std::filesystem::path, FileReport>& file_reports();
    const std::unordered_map<std::filesystem::path, FileReport>& file_reports() const;

private:
    std::unordered_map<std::filesystem::path, FileReport> m_file_reports;
};
}
