#include "progress_bar.hpp"

#include "console_color.hpp"

#include <print>

namespace coverpp
{
static std::string repeat(std::string_view text, std::size_t count)
{
	std::string result;
	result.reserve(text.size() * count);

	for (std::size_t i = 0; i < count; ++i)
	{
		result += text;
	}

	return result;
}

ProgressBar::ProgressBar(std::size_t total, FILE* out, std::size_t width) : out_{out}, total_{total}, width_{width}
{
	update(0);
}

void ProgressBar::update(std::size_t current)
{
	current = std::min(current, total_);

	double const ratio   = total_ == 0 ? 1.0 : static_cast<double>(current) / static_cast<double>(total_);
	auto const   filled  = static_cast<std::size_t>(ratio * static_cast<double>(width_));
	auto const   percent = static_cast<int>(ratio * 100.0);

	std::print(out_,
	           "\r[{}{}{}{}] {:3}% ({}/{})",
	           ColorBold::green,
	           repeat(u8"█", filled),
	           Style::reset,
	           repeat(u8"░", width_ - filled),
	           percent,
	           current,
	           total_);
	std::fflush(out_);
}

void ProgressBar::finish()
{
	update(total_);
	std::println(out_);
}
} // namespace coverpp
