#include "HtmlExporter.hpp"

#include <cmrc/cmrc.hpp>
#include <fstream>
#include <print>

CMRC_DECLARE(coverpp_rc);

namespace
{
struct Line
{
    std::string content;
    unsigned number{};
};

std::ostream& operator<<(std::ostream& os, const Line& line)
{
    std::ranges::copy(
        std::string_view{line.content}
        | std::views::split(std::string_view{"<"})
        | std::views::join_with(std::string_view{"&lt;"})
        | std::ranges::to<std::string>()
        | std::views::split(std::string_view{">"})
        | std::views::join_with(std::string_view{"&gt;"}),
        std::ostream_iterator<char>(os)
    );
    return os << '\n';
}

std::istream& operator>>(std::istream& is, Line& line)
{
    std::getline(is, line.content);
    ++line.number;
    return is;
}
}

namespace coverpp
{
HtmlExporter::HtmlExporter(std::filesystem::path output_directory) : m_dir{std::move(output_directory)}
{}

void HtmlExporter::export_report(const CoverageSink& covered, const CoverageSink& reachable)
{
    std::filesystem::create_directories(m_dir);

    for (const auto& [source_file_path, reachable_tracepoints] : reachable.tracepoints())
    {
        const auto it = covered.tracepoints().find(source_file_path);
        if (it == covered.tracepoints().end())
        {
            // No coverage in entire file
            continue;
        }

        const auto& covered_tracepoints = it->second;

        //TODO relativize source_file against project dir -> include remaining dir path
        const std::filesystem::path output_file_path = m_dir / (source_file_path.filename().concat(".html"));

        std::ifstream source_file{source_file_path};
        std::ofstream output_file{output_file_path, std::ios_base::out | std::ios_base::trunc};
        if (!source_file || !output_file)
        {
            throw std::runtime_error{"Failed to open file"};
        }


        /*// Write header
        const auto resource_filesystem = cmrc::coverpp_rc::get_filesystem();
        const auto index_html_f = resource_filesystem.open("src/resources/html-exporter/index.html");
        const std::string_view index_html{index_html_f.begin(), index_html_f.end()};
        std::ranges::copy(index_html, std::ostream_iterator<char>(output_file));*/

        std::print(output_file, "<pre><code class=\"language-cpp\">");

        unsigned next_line = 1, next_column = 1;
        for (const Tracepoint& reachable_tracepoint : reachable_tracepoints)
        {
            // TODO: Unfortunately, MSVC doesn't emit column information in the PDB :( See https://developercommunity.visualstudio.com/t/Produce-PDB-with-column-informaiton/1409758?space=21&q=column+width

            // We interpret "no data for columnBegin (i.e. == 0)" as "first column"
            const unsigned column_begin = reachable_tracepoint.columnBegin != 0 ? reachable_tracepoint.columnBegin : 1;

            // Need to write up to (excluding) reachable_tracepoint.begin
            // First, write remaining lines
            if (reachable_tracepoint.lineBegin > next_line)
            {
                std::ranges::copy(
                    std::views::istream<Line>(source_file)
                    | std::views::take(reachable_tracepoint.lineBegin - next_line),
                    std::ostream_iterator<Line>(output_file)
                );
                next_line = reachable_tracepoint.lineBegin;
                next_column = 1;
            }

            // Then, write remaining chars up until, but excluding, the column_begin
            std::ranges::copy(
                std::views::istream<char>(source_file)
                | std::views::take(column_begin - next_column),
                std::ostream_iterator<char>(output_file)
            );

            // Now, emit a <mark> tag
            std::print(output_file, "<mark>");



            break;
        }

        std::println(output_file, "</code></pre>>");
    }

    /*

    Generate like:

    <pre>
        <code class="language-cpp">
            #pragma once

            auto add(auto a, <mark class="cov-n">auto b</mark>)
            {
                <mark class="cov-p">partial</mark>
                <mark class="cov-y">return a + b;</mark>
            }
        </code>
    </pre>
     */
}
}
