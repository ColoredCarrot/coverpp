#pragma once

#include <filesystem>

namespace coverpp
{
struct ServeOptions
{
    std::filesystem::path report_path;
    std::filesystem::path coverpp_install_dir;
    std::uint16_t port;
	bool open;
};
}
