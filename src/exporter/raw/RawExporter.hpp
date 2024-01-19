#pragma once

#include "../AbstractFileExporter.hpp"

namespace coverpp
{
class RawExporter : public AbstractFileExporter
{
public:
    explicit RawExporter(std::filesystem::path out_dir);

    using AbstractFileExporter::run;

protected:
    void run(const BasicReport& covered, const BasicReport& reachable, const std::filesystem::path& out_dir) override;
};
}
