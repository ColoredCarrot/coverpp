#include "flat_report.hpp"

namespace coverpp
{
static std::generator<FlatReportEntry>
    flatten(std::filesystem::path const& path, std::vector<Coverpp::Report::PathReportUnion> const& children)
{
	for (auto& child : children)
	{
		if (auto* file_report = child.AsFileReport())
		{
			co_yield FlatReportEntry{
			    .source_file     = path / file_report->path,
			    .reachable_lines = {std::from_range, file_report->reachable_lines},
			    .covered_lines   = {std::from_range, file_report->covered_lines},
			};
		}
		else
		{
			auto* dir_report = child.AsDirectoryReport();

			co_yield std::ranges::elements_of(flatten(path / dir_report->name, dir_report->children));
		}
	}
}

std::generator<FlatReportEntry> flatten_report(Coverpp::Report::CoverageReportT const& report)
{
	for (auto& root : report.roots)
	{
		co_yield std::ranges::elements_of(flatten(root.path, root.children));
	}
}
} // namespace coverpp
