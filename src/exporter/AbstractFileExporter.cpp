#include "AbstractFileExporter.hpp"

#include <utility>

namespace coverpp
{
AbstractFileExporter::AbstractFileExporter(std::filesystem::path out_dir) : m_out_dir{std::move(out_dir)}
{}

void AbstractFileExporter::run(const BasicReport& covered, const BasicReport& reachable)
{
    create_directories(m_out_dir);
    this->run(covered, reachable, m_out_dir);
}
}
