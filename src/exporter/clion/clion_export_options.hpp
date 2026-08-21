#pragma once

#include "../export_options.hpp"

namespace coverpp
{
struct CLionExportOptions : ExportOptions
{
	std::filesystem::path out_dir;
};
} // namespace coverpp
