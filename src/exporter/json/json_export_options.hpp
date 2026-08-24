#pragma once

#include "../export_options.hpp"

namespace coverpp
{
struct JsonExportOptions : ExportOptions
{
	std::filesystem::path out_file;
};
} // namespace coverpp
