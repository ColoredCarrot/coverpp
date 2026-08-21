#include "HtmlExporter.hpp"

#include "../../report/flat_report.hpp"

#include <cmrc/cmrc.hpp>
#include <fstream>
#include <print>
#include <ranges>
#include <iostream>

CMRC_DECLARE(coverpp_rc);

static const std::string_view html_head = R"###(<!doctype html>
<html lang="en">
<head>
    <meta charset="utf-8"/>

    <title>Cover++ Report</title>

    <link rel="stylesheet" href="G:\Voidev\Official\Projects\C++\Cover++\src\resources\html-exporter\third_party/prism.css"/>
    <link rel="stylesheet" href="G:\Voidev\Official\Projects\C++\Cover++\src\resources\html-exporter\index.css"/>
</head>
<body>
)###";

static const std::string_view html_foot = R"###(<script src="G:\Voidev\Official\Projects\C++\Cover++\src\resources\html-exporter\third_party/prism.js"></script>
</body>
</html>
)###";

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
    return os;
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
void HtmlExporter::run(Coverpp::Report::CoverageReportT const& report)
{
    std::filesystem::create_directories(options.out_dir);

    for (const auto& entry : flatten_report(report))
    {
        //TODO relativize source_file against project dir -> include remaining dir path
        const std::filesystem::path output_file_path = options.out_dir / (entry.source_file.filename().concat(".html"));

        std::ifstream source_file{entry.source_file};
        std::ofstream output_file{output_file_path, std::ios_base::out | std::ios_base::trunc};
        if (!source_file || !output_file)
        {
            throw std::runtime_error{"Failed to open file"};
        }

        std::print(output_file, "{}", html_head);

        std::print(output_file, "<pre><code class=\"language-cpp match-braces\">");

        Line line;
        unsigned next_line = 1;
        for (const unsigned reachable_line : entry.reachable_lines)
        {
            // TODO: Unfortunately, MSVC doesn't emit column information in the PDB :( See https://developercommunity.visualstudio.com/t/Produce-PDB-with-column-informaiton/1409758?space=21&q=column+width

            // Need to write up to (excluding) reachable_tracepoint.begin
            // First, write remaining lines
            while (reachable_line > next_line)
            {
                source_file >> line;
                output_file << line << '\n';
                ++next_line;
            }

            // Now, emit a <mark> tag for the line
            const std::string_view mark_class = entry.covered_lines.contains(reachable_line) ? "cov-y" : "cov-n";
            std::print(output_file, "<mark class=\"{}\">", mark_class);
            source_file >> line;
            output_file << line;
            std::print(output_file, "</mark>\n");

            ++next_line;
        }

        // Output any remaining lines
        while (source_file >> line)
        {
            output_file << line << '\n';
        }

        std::println(output_file, "</code></pre>");
        std::print(output_file, "{}", html_foot);
    }
}
}
