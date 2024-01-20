#include "RawExporter.hpp"
#include "../../util/math_util.hpp"
#include "../../util/encodings_util.hpp"

#include <coverage_report_generated.h>

#include <fstream>
#include <ranges>

namespace coverpp
{
RawExporter::RawExporter(std::filesystem::path out_file, std::filesystem::path source_root)
    : m_out_file(std::move(out_file)), m_source_root(std::move(source_root))
{}

static std::filesystem::path find_root(const std::filesystem::path&)
{

}

void RawExporter::run(const BasicReport& covered, const BasicReport& reachable)
{
    static const std::set<unsigned> empty_set{};

    Coverpp::Report::RootT root;
    root.path = m_source_root.u8string();
    root.directory_separator = windows::utf16le_to_utf8(std::filesystem::path::preferred_separator);

    const auto filter_directory_reports =
        std::views::transform([](const Coverpp::Report::PathReportUnion& u) { return u.AsDirectoryReport(); })
        | std::views::filter(std::identity{})
        | std::views::transform(
            [](const Coverpp::Report::DirectoryReportT* p) -> const Coverpp::Report::DirectoryReportT& { return *p; });

    const auto get_file_report = [&](const std::filesystem::path& source_file) -> Coverpp::Report::FileReportT& {
        auto relative = std::filesystem::relative(source_file, m_source_root);
        if (relative.empty())
        {
            throw std::runtime_error{std::format("Source file not in source directory: {}", source_file.u8string())};
        }

        // From the root, we need to traverse down these path_components.
        // The last component is the source file name.
        auto path_components = relative
                               | std::views::transform(
            [](const std::filesystem::path& component) { return component.u8string(); })
                               | std::ranges::to<std::vector>();

        std::vector<Coverpp::Report::PathReportUnion>* children = &root.children;
        for (const auto& component : path_components | std::views::take(path_components.size() - 1))
        {
            auto it = std::ranges::find_if(*children,
                                           [&](const Coverpp::Report::PathReportUnion& u) -> bool {
                                               auto dir = u.AsDirectoryReport();
                                               return dir != nullptr && dir->name == component;
                                           });

            Coverpp::Report::PathReportUnion& r = it != children->end() ? *it : [&]() -> decltype(auto) {
                Coverpp::Report::DirectoryReportT component_report;
                component_report.name = component;
                auto& u = children->emplace_back();
                u.Set(std::move(component_report));
                return u;
            }();
            children = &r.AsDirectoryReport()->children;
        }

        // Finally, find the source file name as an element of children
        auto it = std::ranges::find_if(*children,
                                       [&](const Coverpp::Report::PathReportUnion& u) -> bool {
                                           auto file = u.AsFileReport();
                                           return file != nullptr && file->path == path_components.back();
                                       });

        Coverpp::Report::PathReportUnion& file_report = it != children->end() ? *it : [&]() -> decltype(auto) {
            Coverpp::Report::FileReportT new_file_report;
            new_file_report.path = path_components.back();
            auto& u = children->emplace_back();
            u.Set(std::move(new_file_report));
            return u;
        }();
        return *file_report.AsFileReport();
    };

    for (const auto& [source_file, reachable_lines] : reachable.file_reports())
    {
        const auto it = covered.file_reports().find(source_file);
        const auto& covered_lines = it != covered.file_reports().end() ? it->second.covered_lines() : empty_set;

        auto& file_report = get_file_report(source_file);

        file_report.reachable_lines = reachable_lines.covered_lines() | std::ranges::to<std::vector>();
        file_report.covered_lines = covered_lines | std::ranges::to<std::vector>();
    }

    Coverpp::Report::CoverageReportT result;
    result.roots.push_back(std::move(root));

    flatbuffers::FlatBufferBuilder builder{1024};
    builder.Finish(Coverpp::Report::CoverageReport::Pack(builder, &result));

    std::filesystem::create_directories(m_out_file.parent_path());
    std::ofstream file{m_out_file, std::ios_base::out | std::ios_base::trunc | std::ios_base::binary};
    file.exceptions(std::ios_base::badbit | std::ios_base::failbit);
    file.write(reinterpret_cast<const char*>(builder.GetBufferPointer()), builder.GetSize());
}
}
