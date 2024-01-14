#include "SourceFileReportGenerator.hpp"

#include <cstdio>
#include <print>
#include <fstream>
#include <ranges>

namespace coverpp
{
SourceFileReportGenerator::SourceFileReportGenerator(std::filesystem::path output_directory)
    : m_dir{std::move(output_directory)}
{}

struct FILE_Deleter
{
    void operator()(FILE* f)
    {
        std::fclose(f);
    }
};

using unique_file = std::unique_ptr<FILE, FILE_Deleter>;

namespace
{
struct Line
{
    std::string content;
    unsigned number{};
};

std::istream& operator>>(std::istream& is, Line& line)
{
    std::getline(is, line.content);
    ++line.number;
    return is;
}
}

void SourceFileReportGenerator::generate_report(const BasicReport& report, const BasicReport& reachability_report)
{
    std::filesystem::create_directories(m_dir);

    for (const auto& [source_file_path, reachability_file_report] : reachability_report.file_reports())
    {
        const auto it = report.file_reports().find(source_file_path);
        if (it == report.file_reports().end())
        {
            // No coverage in entire file
            continue;
        }

        const auto& file_report = it->second;

        //TODO relativize source_file against project dir -> include remaining dir path
        const std::filesystem::path output_file_path = m_dir / source_file_path.filename();

        std::ifstream source_file{source_file_path};
        unique_file output_file{std::fopen(output_file_path.string().c_str(), "w")};
        if (!source_file || !output_file)
        {
            throw std::runtime_error{"Failed to open file"};
        }

        for (const Line& line : std::views::istream<Line>(source_file))
        {
            if (reachability_file_report.covered_lines().contains(line.number))
            {
                if (file_report.covered_lines().contains(line.number))
                {
                    std::println(output_file.get(), "/* : */ {}", line.content);
                }
                else
                {
                    std::println(output_file.get(), "/*-X-*/ {}", line.content);
                }
            }
            else
            {
                std::println(output_file.get(), "        {}", line.content);
            }
        }
    }
}
}
