#pragma once

#include <iostream>

namespace coverpp
{
class ProgressBar
{
public:
	explicit ProgressBar(std::size_t total, std::ostream& out = std::cout, std::size_t width = 40);

	void update(std::size_t current);

	void finish();

private:
	std::ostream* out_;
	std::size_t   total_;
	std::size_t   width_;
};
} // namespace coverpp
