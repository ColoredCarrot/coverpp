#pragma once

#include <filesystem>

namespace coverpp
{
struct MergeOptions
{
	std::vector<std::filesystem::path> input_files;
	std::filesystem::path              output_file;
};
} // namespace coverpp
