#pragma once

#include <filesystem>

namespace coverpp
{
struct ExportOptions
{
	std::filesystem::path report_file;
};
} // namespace coverpp
