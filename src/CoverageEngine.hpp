#pragma once

namespace coverpp {
class CoverageEngine {
public:
    virtual ~CoverageEngine() = default;

    virtual void run_with_coverage(const std::filesystem)
};
}
