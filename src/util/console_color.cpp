#include "console_color.hpp"

#include <mutex>
#include <ostream>

#define NOMINMAX

#include <Windows.h>
#include <wil/result_macros.h>

#define INSTANTIATIONS() INSTANTIATE(Style) INSTANTIATE(Color) INSTANTIATE(BgColor) INSTANTIATE(ColorBold) INSTANTIATE(BgColorBold)

namespace coverpp
{

static void enable_color_support()
{
    static bool color_support_enabled{false};
    static std::mutex mutex;

    if (color_support_enabled)
    {
        return;
    }

    std::lock_guard guard{mutex};

    if (color_support_enabled)
    {
        return;
    }
    color_support_enabled = true;

    const auto enable_color = [](HANDLE h) {
        THROW_LAST_ERROR_IF(h == INVALID_HANDLE_VALUE); // NOLINT(*-lambda-function-name)

        DWORD mode;
        THROW_LAST_ERROR_IF(not GetConsoleMode(h, &mode)); // NOLINT(*-lambda-function-name)

        mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;

        THROW_LAST_ERROR_IF(not SetConsoleMode(h, mode)); // NOLINT(*-lambda-function-name)
    };

    enable_color(GetStdHandle(STD_OUTPUT_HANDLE));
    enable_color(GetStdHandle(STD_ERROR_HANDLE));
}

template<ColorControl T>
std::ostream& operator<<(std::ostream& os, T control)
{
    enable_color_support();
    return os << "\033[" << std::to_underlying(control) << "m";
}

#define INSTANTIATE(T) template std::ostream& operator<< <T>(std::ostream& os, T control);
INSTANTIATIONS()
#undef INSTANTIATE
}

template<coverpp::ColorControl T>
std::format_context::iterator std::formatter<T>::format(T control, std::format_context& ctx) const
{
    coverpp::enable_color_support();
    return std::format_to(ctx.out(), "\033[{}m", std::to_underlying(control));
}


#define INSTANTIATE(T) template struct std::formatter<coverpp::T>;
INSTANTIATIONS() // NOLINT(*-dcl58-cpp)
#undef INSTANTIATE
