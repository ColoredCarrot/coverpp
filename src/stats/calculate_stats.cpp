#include "calculate_stats.hpp"

namespace coverpp
{
static Coverpp::Report::StatsT& operator+=(Coverpp::Report::StatsT& stats, Coverpp::Report::StatsT const& more)
{
	stats.total_lines += more.total_lines;
	stats.total_reachable += more.total_reachable;
	stats.total_covered += more.total_covered;
	return stats;
}

static std::unique_ptr<Coverpp::Report::StatsT> calculate_stats(std::vector<Coverpp::Report::PathReportUnion>& reports)
{
	auto stats = std::make_unique<Coverpp::Report::StatsT>();
	for (auto& report : reports)
	{
		if (auto* directory_report = report.AsDirectoryReport())
		{
			auto child_stats = calculate_stats(directory_report->children);
			*stats += *child_stats;
			directory_report->stats = std::move(child_stats);
		}
		else if (auto* child_file_report = report.AsFileReport())
		{
			stats->total_lines += child_file_report->total_lines;
			stats->total_reachable += child_file_report->reachable_lines.size();
			stats->total_covered += child_file_report->covered_lines.size();
		}
	}
	return stats;
}

void calculate_stats(Coverpp::Report::RootT& root)
{
	root.stats = calculate_stats(root.children);
}

void calculate_stats(Coverpp::Report::CoverageReportT& report)
{
	report.stats = std::make_unique<Coverpp::Report::StatsT>();
	for (auto& root : report.roots)
	{
		calculate_stats(root);
		*report.stats += *root.stats;
	}
}
} // namespace coverpp
