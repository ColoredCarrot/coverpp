#include "console_color.hpp"

#include <mutex>
#include <ostream>

#define NOMINMAX

#include <Windows.h>
#include <wil/result_macros.h>

#define INSTANTIATIONS() INSTANTIATE(Style) INSTANTIATE(Color) INSTANTIATE(BgColor) INSTANTIATE(ColorBold) INSTANTIATE(BgColorBold)

namespace coverpp
{

enum class ColorSupportStatus
{
    uninitialized,
    on,
    off,
};

static bool enable_color_support()
{
    using
    enum ColorSupportStatus;

    static ColorSupportStatus color_support_enabled{uninitialized};
    static std::mutex mutex;

    if (color_support_enabled != uninitialized)
    {
        return color_support_enabled == on;
    }

    std::lock_guard guard{mutex};

    if (color_support_enabled != uninitialized)
    {
        return color_support_enabled == on;
    }
    color_support_enabled = on;

    const auto enable_color = [](HANDLE h) {
        THROW_LAST_ERROR_IF(h == INVALID_HANDLE_VALUE); // NOLINT(*-lambda-function-name)

        DWORD mode;
        if (!GetConsoleMode(h, &mode))
        {
            color_support_enabled = off;
            return;
        }

        mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;

        SetConsoleMode(h, mode);
    };

    enable_color(GetStdHandle(STD_OUTPUT_HANDLE));
    enable_color(GetStdHandle(STD_ERROR_HANDLE));

	return color_support_enabled == on;
}

template<ColorControl T>
std::ostream& operator<<(std::ostream& os, T control)
{
    if (enable_color_support())
    {
    	os << "\033[" << std::to_underlying(control) << "m";
    }
    return os;
}

#define INSTANTIATE(T) template std::ostream& operator<< <T>(std::ostream& os, T control);
INSTANTIATIONS()
#undef INSTANTIATE
}

template<coverpp::ColorControl T>
std::format_context::iterator std::formatter<T>::format(T control, std::format_context& ctx) const
{
	if (coverpp::enable_color_support())
    {
		return std::format_to(ctx.out(), "\033[{}m", std::to_underlying(control));
    }
	else
    {
	    return ctx.out();
    }
}


#define INSTANTIATE(T) template struct std::formatter<coverpp::T>;
INSTANTIATIONS() // NOLINT(*-dcl58-cpp)
#undef INSTANTIATE
