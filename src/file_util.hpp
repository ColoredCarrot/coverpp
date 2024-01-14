#pragma once

#include <filesystem>

namespace coverpp::detail
{
bool path_is_subpath_of(const std::filesystem::path& sub_path, const std::filesystem::path& base_path);
}
