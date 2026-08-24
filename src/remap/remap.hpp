#pragma once

#include <filesystem>

namespace coverpp
{
struct RemapOptions
{
	std::filesystem::path report;
	std::filesystem::path from;
	std::filesystem::path to;
};

int remap(RemapOptions const& options);
}
