#include "JsonExporter.hpp"

#include "../../report/flat_report.hpp"
#include "../../util/guard.hpp"

#include <rapidjson/rapidjson.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/prettywriter.h>

#include <iostream>

namespace coverpp
{
namespace
{
class JsonOut
{
public:
	explicit JsonOut(FILE* out) : m_buf(64 * 1024), m_stream(out, m_buf.data(), m_buf.size()), m_out(m_stream)
	{
		m_out.SetIndent(' ', 2);
	}

	void process(Coverpp::Report::CoverageReportT const& report)
	{
		m_out.StartObject();

		m_out.Key("roots");
		m_out.StartArray();
		for (auto& root : report.roots)
		{
			process(root);
		}
		m_out.EndArray();

		m_out.EndObject();
	}

	void process(Coverpp::Report::RootT const& root)
	{
		m_out.StartObject();

		m_out.Key("path");
		m_out.String(root.path.data(), root.path.length());

		process(*root.stats);

		m_out.Key("children");
		process(root.children);

		m_out.EndObject();
	}

	void process(Coverpp::Report::StatsT const& stats)
	{
		m_out.Key("total");
		m_out.Uint64(stats.total_lines);
		m_out.Key("reachable");
		m_out.Uint64(stats.total_reachable);
		m_out.Key("covered");
		m_out.Uint64(stats.total_covered);
	}

	void process(std::vector<Coverpp::Report::PathReportUnion> const& children)
	{
		m_out.StartArray();

		for (auto& child : children)
		{
			if (auto file_report = child.AsFileReport())
			{
				process(*file_report);
			}
			else if (auto directory_report = child.AsDirectoryReport())
			{
				process(*directory_report);
			}
		}

		m_out.EndArray();
	}

	void process(Coverpp::Report::FileReportT const& file_report)
	{
		m_out.StartObject();

		m_out.Key("file");
		m_out.String(file_report.path.data(), file_report.path.length());

		m_out.Key("total");
		m_out.Uint64(file_report.total_lines);
		m_out.Key("reachable");
		m_out.Uint64(file_report.reachable_lines.size());
		m_out.Key("covered");
		m_out.Uint64(file_report.covered_lines.size());

		m_out.EndObject();
	}

	void process(Coverpp::Report::DirectoryReportT const& directory_report)
	{
		m_out.StartObject();

		m_out.Key("directory");
		m_out.String(directory_report.name.data(), directory_report.name.length());

		process(*directory_report.stats);

		m_out.Key("children");
		process(directory_report.children);

		m_out.EndObject();
	}

private:
	std::vector<char>                                   m_buf;
	rapidjson::FileWriteStream                          m_stream;
	rapidjson::PrettyWriter<rapidjson::FileWriteStream> m_out;
};
} // namespace

void JsonExporter::run(Coverpp::Report::CoverageReportT const& report)
{
	bool const is_stdout = options.out_file == "-";
	auto const raw_out   = is_stdout ? stdout : std::fopen(options.out_file.string().c_str(), "w");
	if (!raw_out)
	{
		throw std::runtime_error{std::format("Failed to open file: {}", options.out_file.string())};
	}
	auto _ = Guard{[is_stdout, raw_out] {
		if (!is_stdout)
		{
			std::fclose(raw_out);
		}
	}};

	auto out = JsonOut{raw_out};

	out.process(report);
}
} // namespace coverpp
