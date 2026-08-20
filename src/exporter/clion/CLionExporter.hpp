#pragma once

#include "../Exporter.hpp"

namespace coverpp
{
class CLionExporter : public Exporter
{
public:
	CLionExporter(std::filesystem::path out_dir, std::filesystem::path source_root);

	void run(BasicReport const& covered, BasicReport const& reachable) override;

private:
	std::filesystem::path m_out_dir;
	std::filesystem::path m_source_root;
};
} // namespace coverpp
