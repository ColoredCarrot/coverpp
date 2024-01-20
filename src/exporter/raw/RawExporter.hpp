#pragma once

#include "../Exporter.hpp"

namespace coverpp
{
class RawExporter : public Exporter
{
public:
    RawExporter(std::filesystem::path out_file, std::filesystem::path source_root);

    void run(const BasicReport& covered, const BasicReport& reachable) override;

private:
    std::filesystem::path m_out_file;
    std::filesystem::path m_source_root;
};
}
