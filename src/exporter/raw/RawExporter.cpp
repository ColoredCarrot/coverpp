#include "RawExporter.hpp"
#include "../../util/math_util.hpp"

#include <coverage_report_generated.h>

#include <fstream>
#include <ranges>

namespace coverpp
{
RawExporter::RawExporter(std::filesystem::path out_dir) : AbstractFileExporter(std::move(out_dir))
{}

void RawExporter::run(const BasicReport& covered, const BasicReport& reachable, const std::filesystem::path& out_dir)
{
    static const std::set<unsigned> empty_set{};

    std::filesystem::path file_path = out_dir / "report.coverpp";

    flatbuffers::FlatBufferBuilder builder{1024};
    std::vector<flatbuffers::Offset<Coverpp::FileReport>> file_reports;
    for (const auto& [source_file, reachable_lines] : reachable.file_reports())
    {
        const auto it = covered.file_reports().find(source_file);
        const auto& covered_lines = it != covered.file_reports().end() ? it->second.covered_lines() : empty_set;

        auto source_file_path = builder.CreateString(source_file.u8string());

        auto reachable_lines_v = builder.CreateVector(reachable_lines.covered_lines() | std::ranges::to<std::vector>());
        auto covered_lines_v = builder.CreateVector(covered_lines | std::ranges::to<std::vector>());

        file_reports.push_back(
            Coverpp::CreateFileReport(builder, source_file_path, reachable_lines_v, covered_lines_v));
    }
    auto coverage_report = Coverpp::CreateCoverageReportDirect(builder, &file_reports);
    builder.Finish(coverage_report);

    std::ofstream file{file_path, std::ios_base::out | std::ios_base::trunc | std::ios_base::binary};
    file.exceptions(std::ios_base::badbit | std::ios_base::failbit);
    file.write(reinterpret_cast<const char*>(builder.GetBufferPointer()), builder.GetSize());
}
}
