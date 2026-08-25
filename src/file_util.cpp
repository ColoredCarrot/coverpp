#include "file_util.hpp"

#include <array>
#include <fstream>

namespace coverpp::detail
{
bool path_is_subpath_of(std::filesystem::path const& sub_path, std::filesystem::path const& base_path)
{
	auto const r = std::ranges::mismatch(base_path, sub_path);
	return r.in1 == base_path.end();
}

std::uint32_t lines_in_file(std::filesystem::path const& file)
{
	std::ifstream in{file};
	in.exceptions(std::ios_base::badbit);

	std::array<char, 10 * 1024> buffer; // NOLINT(*-pro-type-member-init)

	std::uint32_t lines = 0;
	while (in)
	{
		in.read(buffer.data(), buffer.size());
		auto const n = in.gcount();

		lines += std::ranges::count(buffer.data(), buffer.data() + n, '\n');
	}

	return lines;
}

std::optional<std::string> read_file_binary(std::filesystem::path const& file)
{
	std::ifstream ifs(file, std::ios_base::in | std::ios_base::binary | std::ios_base::ate);
	if (!ifs)
	{
		return std::nullopt;
	}

	auto const size = ifs.tellg();
	ifs.seekg(0, std::ios::beg);

	std::vector<char> bytes(size);
	ifs.read(bytes.data(), size);

	return std::string{bytes.data(), static_cast<std::size_t>(size)};
}
} // namespace coverpp::detail
