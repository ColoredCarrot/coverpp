#include "remap.hpp"

#include "../report/report_file_utils.hpp"
#include "../stats/calculate_stats.hpp"

#include <coverage_report_generated.h>

#include <algorithm>
#include <optional>
#include <ranges>
#include <span>
#include <stdexcept>
#include <vector>

namespace coverpp
{
namespace
{
using Children = std::vector<Coverpp::Report::PathReportUnion>;

bool starts_with(std::filesystem::path const& path, std::filesystem::path const& prefix)
{
	auto path_it   = path.begin();
	auto prefix_it = prefix.begin();

	while (prefix_it != prefix.end())
	{
		if (path_it == path.end() || *path_it != *prefix_it)
		{
			return false;
		}

		++path_it;
		++prefix_it;
	}

	return true;
}

std::filesystem::path replace_prefix(std::filesystem::path const& path,
                                     std::filesystem::path const& from,
                                     std::filesystem::path const& to)
{
	auto const suffix = path.lexically_relative(from);
	return suffix.empty() || suffix == "." ? to : (to / suffix).lexically_normal();
}

std::optional<Children> extract_children(Children& children, std::span<std::filesystem::path const> components)
{
	if (components.empty())
	{
		return std::nullopt;
	}

	auto const child_it = std::ranges::find_if(children, [&](Coverpp::Report::PathReportUnion& child) {
		auto const* directory = child.AsDirectoryReport();
		return directory && std::filesystem::path{directory->name} == components.front();
	});
	if (child_it == children.end())
	{
		return std::nullopt;
	}
	auto* directory = child_it->AsDirectoryReport();

	if (components.size() == 1)
	{
		auto extracted = std::move(directory->children);
		children.erase(child_it);
		return extracted;
	}

	auto extracted = extract_children(directory->children, components.subspan(1));
	if (extracted && directory->children.empty())
	{
		children.erase(child_it);
	}

	return extracted;
}
} // namespace

int remap(RemapOptions const& options)
{
	auto report = read_report(options.report);

	if (report.roots.empty())
	{
		calculate_stats(report);
		write_report(options.report, report);
		return 0;
	}

	auto from = options.from;
	if (from.empty())
	{
		if (report.roots.size() > 1)
		{
			throw std::runtime_error{"Cannot remap from multiple roots unless a specific root is specified"};
		}

		from = report.roots.front().path;
	}

	from          = std::filesystem::weakly_canonical(from);
	auto const to = std::filesystem::weakly_canonical(options.to);

	std::vector<Coverpp::Report::RootT> remapped_roots;
	remapped_roots.reserve(report.roots.size() + 1);

	for (auto& root : report.roots)
	{
		auto const root_path = std::filesystem::weakly_canonical(std::filesystem::path{root.path});

		if (starts_with(root_path, from))
		{
			root.path = replace_prefix(root_path, from, to).u8string();
			remapped_roots.push_back(std::move(root));
			continue;
		}

		if (!starts_with(from, root_path))
		{
			remapped_roots.push_back(std::move(root));
			continue;
		}

		auto const relative   = from.lexically_relative(root_path);
		auto const components = relative | std::ranges::to<std::vector<std::filesystem::path>>();

		auto extracted = extract_children(root.children, components);
		if (!extracted)
		{
			remapped_roots.push_back(std::move(root));
			continue;
		}

		auto separator = root.directory_separator;

		if (!root.children.empty())
		{
			remapped_roots.push_back(std::move(root));
		}

		if (!extracted->empty())
		{
			Coverpp::Report::RootT mapped_root;
			mapped_root.path                = to.u8string();
			mapped_root.directory_separator = std::move(separator);
			mapped_root.children            = std::move(*extracted);
			remapped_roots.push_back(std::move(mapped_root));
		}
	}

	report.roots = std::move(remapped_roots);

	calculate_stats(report);
	write_report(options.report, report);
	return 0;
}
} // namespace coverpp
