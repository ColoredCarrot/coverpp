#pragma once

#include "Exporter.hpp"

namespace coverpp
{
class AbstractFileExporter : public Exporter
{
public:
    explicit AbstractFileExporter(std::filesystem::path out_dir);

    void run(const BasicReport& covered, const BasicReport& reachable) override;

protected:
    virtual void run(
        const BasicReport& covered,
        const BasicReport& reachable,
        const std::filesystem::path& out_dir
    ) = 0;

private:
    std::filesystem::path m_out_dir;
};
}
