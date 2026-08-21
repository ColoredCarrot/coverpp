#pragma once

#include "../export_options.hpp"

namespace coverpp
{
struct HtmlExportOptions : ExportOptions
{
	std::filesystem::path out_dir;
};
} // namespace coverpp
