#pragma once

#include <filesystem>

namespace coverpp
{
struct ServeOptions
{
    std::filesystem::path report_path;
    int port;
};
}
