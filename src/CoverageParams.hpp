#pragma once

#include <filesystem>
#include <regex>

namespace coverpp
{
struct CoverageParams
{
    std::filesystem::path source_dir;
    std::filesystem::path program;
    std::filesystem::path debug_info;
    std::string_view program_args;

	std::regex exclude_source_files_regex;

    std::filesystem::path out_file;

    int verbosity;
    bool print_first_chance_seh_exceptions;
};
}
