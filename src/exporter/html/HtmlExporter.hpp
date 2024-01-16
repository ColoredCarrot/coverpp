#pragma once

#include "../Exporter.hpp"

namespace coverpp
{
class HtmlExporter : public Exporter
{
public:
    explicit HtmlExporter(std::filesystem::path output_directory);

    void run(const BasicReport& covered, const BasicReport& reachable) override;

private:
    std::filesystem::path m_dir;
};
}
