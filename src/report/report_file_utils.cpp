#include "report_file_utils.hpp"

#include <fstream>

namespace coverpp
{
Coverpp::Report::CoverageReportT read_report(std::filesystem::path const& file)
{
	std::ifstream stream{file, std::ios::binary | std::ios::in};

	stream.seekg(0, std::ios::end);
	auto const length = stream.tellg();
	stream.seekg(0, std::ios::beg);

	auto data = std::vector<char>(length);
	stream.read(data.data(), length);
	stream.close();

	Coverpp::Report::CoverageReportT report;
	Coverpp::Report::GetCoverageReport(data.data())->UnPackTo(&report);

	return report;
}

void write_report(std::filesystem::path const& file, Coverpp::Report::CoverageReportT const& report)
{
	flatbuffers::FlatBufferBuilder builder{1024};
	builder.Finish(Coverpp::Report::CoverageReport::Pack(builder, &report));

	std::filesystem::create_directories(file.parent_path());
	std::ofstream stream{file, std::ios_base::out | std::ios_base::trunc | std::ios_base::binary};
	stream.exceptions(std::ios_base::badbit | std::ios_base::failbit);
	stream.write(reinterpret_cast<char const*>(builder.GetBufferPointer()), builder.GetSize());
}
} // namespace coverpp
