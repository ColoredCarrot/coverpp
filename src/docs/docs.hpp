#pragma once

#include <filesystem>

namespace coverpp
{
struct DocsOptions
{
	std::filesystem::path coverpp_install_dir;
	std::uint16_t         port;
	bool                  open;
};

int serve_docs(DocsOptions const& options);
} // namespace coverpp
