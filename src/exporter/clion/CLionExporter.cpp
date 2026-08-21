#include "CLionExporter.hpp"

#include "../../report/flat_report.hpp"
#include "../../util/encodings_util.hpp"

#include <fstream>
#include <iostream>
#include <ranges>
#include <print>

namespace coverpp
{
void CLionExporter::run(Coverpp::Report::CoverageReportT const& report)
{
	std::filesystem::create_directories(options.out_dir);

	auto const flat = flatten_report(report) | std::ranges::to<std::vector>();

	auto index = 0uz;
	auto width = static_cast<int>(std::log10(flat.size())) + 1;
	for (auto const& entry : flat)
	{
		auto out_file = options.out_dir / std::format("{:0{}}.gcov", index++, width);

		std::ofstream out{out_file, std::ios_base::out | std::ios_base::trunc | std::ios_base::binary};
		out.exceptions(std::ios_base::badbit | std::ios_base::failbit);

		std::println(out, "    -:    0:Source:{}", absolute(entry.source_file).generic_string());
		std::println(out, "    -:    0:Runs:1");

		std::ifstream in{entry.source_file};
		in.exceptions(std::ios_base::badbit);

		std::string line;
		for (std::size_t line_number = 1; std::getline(in, line); ++line_number)
		{
			bool is_reachable = entry.reachable_lines.contains(line_number);
			bool is_covered   = entry.covered_lines.contains(line_number);

			std::println(out,
			             "{}:{: 5}:{}",
			             is_covered     ? "    1"
			             : is_reachable ? "#####"
			                            : "    -",
			             line_number,
			             line);
		}

		out.flush();
	}
}
} // namespace coverpp
