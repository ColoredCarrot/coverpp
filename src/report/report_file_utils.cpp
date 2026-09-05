#include "report_file_utils.hpp"

#include <fstream>

namespace coverpp
{
using format_version_t = std::int32_t;

static constexpr std::string_view magic = "CvPP\xC0\x99";

static_assert(std::endian::native == std::endian::little || std::endian::native == std::endian::big);

/*
 * coverpp files follow this format:
 *  - Magic (will never change)
 *  - Format version: 32-bit integer (little endian) (currently always 1)
 *  - Payload: FlatBuffers buffer
 */

Coverpp::Report::CoverageReportT read_report(std::filesystem::path const& file)
{
	std::ifstream stream{file, std::ios::binary | std::ios::in};

	if (!stream)
	{
		throw std::runtime_error("Failed to open file: " + file.string());
	}

	stream.seekg(0, std::ios::end);
	auto const length = stream.tellg();
	stream.seekg(0, std::ios::beg);

	auto data = std::vector<char>(length);
	stream.read(data.data(), length);
	stream.close();

	if (length < magic.length() + sizeof(format_version_t))
	{
		throw std::runtime_error("Malformed coverage report: " + file.string());
	}

	// Magic
	if (std::string_view{data.data(), data.data() + magic.length()} != magic)
	{
		throw std::runtime_error("Malformed coverage report: " + file.string());
	}

	// Format version
	format_version_t version;
	std::memcpy(&version, data.data() + magic.length(), sizeof(format_version_t));
	if constexpr (std::endian::native != std::endian::little)
	{
		version = std::byteswap(version);
	}
	if (version != 1)
	{
		throw std::runtime_error(std::format("Unsupported coverage report format version: {} in {}", version, file.string()));
	}

	// Payload
	Coverpp::Report::CoverageReportT report;
	Coverpp::Report::GetCoverageReport(data.data() + magic.length() + sizeof(format_version_t))->UnPackTo(&report);

	return report;
}

void write_report(std::filesystem::path const& file, Coverpp::Report::CoverageReportT const& report)
{
	flatbuffers::FlatBufferBuilder builder{1024};
	builder.Finish(Coverpp::Report::CoverageReport::Pack(builder, &report));

	std::filesystem::create_directories(file.parent_path());
	std::ofstream stream{file, std::ios_base::out | std::ios_base::trunc | std::ios_base::binary};
	stream.exceptions(std::ios_base::badbit | std::ios_base::failbit);

	// Magic
	stream.write(magic.data(), magic.length());

	// Format version
	format_version_t format_version = 1;
	if constexpr (std::endian::native != std::endian::little)
	{
		format_version = std::byteswap(format_version);
	}
	stream.write(reinterpret_cast<char const*>(&format_version), sizeof(format_version));

	// Payload
	stream.write(reinterpret_cast<char const*>(builder.GetBufferPointer()), builder.GetSize());
}
} // namespace coverpp
