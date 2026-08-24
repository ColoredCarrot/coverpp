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
} // namespace coverpp::detail
