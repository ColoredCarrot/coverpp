#pragma once

#include <filesystem>

namespace coverpp
{
struct CoverageParams
{
    std::filesystem::path source_dir;
    std::filesystem::path program;
    std::filesystem::path debug_info;
    std::string_view program_args;

    std::filesystem::path out_dir;

    int verbosity;
    bool print_first_chance_seh_exceptions;
};
}
