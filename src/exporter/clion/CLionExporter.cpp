#include "CLionExporter.hpp"
#include "../../util/math_util.hpp"
#include "../../util/encodings_util.hpp"

#include <fstream>
#include <iostream>
#include <ranges>
#include <print>

namespace coverpp
{
CLionExporter::CLionExporter(std::filesystem::path out_dir, std::filesystem::path source_root)
    : m_out_dir(std::move(out_dir)), m_source_root(std::move(source_root))
{}

void CLionExporter::run(BasicReport const& covered, BasicReport const& reachable)
{
	std::filesystem::create_directories(m_out_dir);

	auto index = 0uz;
	auto width = static_cast<int>(std::log10(reachable.file_reports().size())) + 1;
	for (auto const& [path, file_report] : reachable.file_reports())
	{
		auto const& covered_file_report = covered.file_reports().at(path);

		auto out_file = m_out_dir / std::format("{:0{}}.gcov", index++, width);

		std::ofstream out{out_file, std::ios_base::out | std::ios_base::trunc | std::ios_base::binary};
		out.exceptions(std::ios_base::badbit | std::ios_base::failbit);

		std::println(out, "    -:    0:Source:{}", absolute(path).generic_string());
		std::println(out, "    -:    0:Runs:1");

		std::ifstream in{path};
		in.exceptions(std::ios_base::badbit);

		std::string line;
		for (std::size_t line_number = 1; std::getline(in, line); ++line_number)
		{
			bool is_reachable = file_report.covered_lines().contains(line_number);
			bool is_covered   = is_reachable && covered_file_report.covered_lines().contains(line_number);

			std::println(out, "{}:{: 5}:{}", is_covered ? "    1" : is_reachable ? "#####" : "    -", line_number, line);
		}

		out.flush();
	}
}
} // namespace coverpp
