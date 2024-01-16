#include "RawExporter.hpp"
#include "../../util/math_util.hpp"

#include <fstream>
#include <string>

namespace coverpp
{
RawExporter::RawExporter(std::filesystem::path out_dir) : AbstractFileExporter(std::move(out_dir))
{}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "Simplify"
template<std::integral T>
static T to_little_endian(T value)
{
    if constexpr (std::endian::native == std::endian::little)
    {
        return value;
    }
    else if constexpr (std::endian::native == std::endian::big)
    {
        return std::byteswap(value);
    }
    else
    {
        static_assert(sizeof(T) == 1, "Mixed-endian environments are not supported");
        return value;
    }
}
#pragma clang diagnostic pop

#if defined(__JETBRAINS_IDE__) || defined(__INTELLISENSE__)
static const char* utf8_data(const std::u8string& s)
{
    return reinterpret_cast<const char*>(s.data());
}
#else
static const char* utf8_data(const std::string& s)
{
    return s.data();
}
#endif
void RawExporter::run(const BasicReport& covered, const BasicReport& reachable, const std::filesystem::path& out_dir)
{
    static const std::set<unsigned> empty_set{};

    std::filesystem::path file_path = out_dir / "report.coverpp";

    std::ofstream file{file_path, std::ios_base::out | std::ios_base::trunc | std::ios_base::binary};

    const auto write_integral = [&file](std::integral auto value) {
        value = to_little_endian(value);
        file.write(reinterpret_cast<const char*>(&value), sizeof(value));
    };

    file << start_magic;

    for (const auto& [source_file, reachable_lines] : reachable.file_reports())
    {
        const auto it = covered.file_reports().find(source_file);
        const auto& covered_lines = it != covered.file_reports().end() ? it->second.covered_lines() : empty_set;

        // Write source file path (UTF-8) with length prefix
        const auto source_file_path = source_file.u8string();
        write_integral(source_file_path.size());
        file.write(utf8_data(source_file_path), detail::convert_or_clamp<std::streamsize>(source_file_path.size()));

        // Write reachable lines with total prefix
        write_integral(reachable_lines.covered_lines().size());
        for (unsigned line : reachable_lines.covered_lines())
        {
            write_integral(line);
        }

        // Write covered lines with total prefix
        write_integral(covered_lines.size());
        for (unsigned line : covered_lines)
        {
            write_integral(line);
        }
    }

    file << end_magic;
}
}
