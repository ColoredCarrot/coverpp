#pragma once

#include <unordered_map>
#include <set>
#include <filesystem>

namespace coverpp
{
class BasicFileReport
{
public:
    std::set<unsigned>& covered_lines();
    const std::set<unsigned>& covered_lines() const;

	static const BasicFileReport empty;

private:
    std::set<unsigned> m_covered_lines;
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
