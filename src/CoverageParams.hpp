#pragma once

#include <filesystem>

namespace coverpp
{
struct CoverageParams
{
    std::filesystem::path source_dir;
    std::filesystem::path program;
    std::filesystem::path debug_info;

    std::filesystem::path out_dir;

    int verbosity;
};
}
