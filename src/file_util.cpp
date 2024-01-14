#include "file_util.hpp"

namespace coverpp::detail
{
bool path_is_subpath_of(const std::filesystem::path& sub_path, const std::filesystem::path& base_path)
{
    const auto r = std::ranges::mismatch(base_path, sub_path);
    return r.in1 == base_path.end();
}
}
