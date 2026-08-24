#pragma once

#include <filesystem>

namespace coverpp::detail
{
bool path_is_subpath_of(std::filesystem::path const& sub_path, std::filesystem::path const& base_path);

/** Returns 0 if the file is not readable */
std::uint32_t lines_in_file(std::filesystem::path const& file);

} // namespace coverpp::detail
