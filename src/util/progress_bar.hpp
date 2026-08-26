#pragma once

#include <iostream>

namespace coverpp
{
class ProgressBar
{
public:
	explicit ProgressBar(std::size_t total, FILE* out = stdout, std::size_t width = 40);

	void update(std::size_t current);

	void finish();

private:
	FILE*       out_;
	std::size_t total_;
	std::size_t width_;
};
} // namespace coverpp
