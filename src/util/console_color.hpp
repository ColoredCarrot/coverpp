#pragma once

#include <concepts>
#include <format>

namespace coverpp
{
enum class Style
{
    reset = 0,
    bold = 1,
    dim = 2,
    italic = 3,
    underline = 4,
    blink = 5,
    rblink = 6,
    reversed = 7,
    conceal = 8,
    crossed = 9,
};

enum class Color
{
    black = 30,
    red = 31,
    green = 32,
    yellow = 33,
    blue = 34,
    magenta = 35,
    cyan = 36,
    gray = 37,
    reset = 39,
};

enum class BgColor
{
    black = 40,
    red = 41,
    green = 42,
    yellow = 43,
    blue = 44,
    magenta = 45,
    cyan = 46,
    gray = 47,
    reset = 49,
};

enum class ColorBold
{
    black = 90,
    red = 91,
    green = 92,
    yellow = 93,
    blue = 94,
    magenta = 95,
    cyan = 96,
    gray = 97,
};

enum class BgColorBold
{
    black = 100,
    red = 101,
    green = 102,
    yellow = 103,
    blue = 104,
    magenta = 105,
    cyan = 106,
    gray = 107,
};

template<typename T>
concept ColorControl = std::same_as<T, Style> || std::same_as<T, Color> || std::same_as<T, BgColor> ||
                       std::same_as<T, ColorBold> || std::same_as<T, BgColorBold>;

template<typename T, ColorControl auto... Cs>
class Styled
{
public:
    template<typename U>
    requires std::constructible_from<T, U>
    explicit Styled(U&& payload) : payload{std::forward<U>(payload)}
    {}

    T payload;
};

template<ColorControl auto... Cs, typename T>
auto styled(T&& payload)
{
    return Styled<T, Cs...>{std::forward<T>(payload)};
}


template<ColorControl T>
std::ostream& operator<<(std::ostream& os, T control);
}

template<coverpp::ColorControl T>
struct std::formatter<T> // NOLINT(*-dcl58-cpp)
{
    constexpr auto parse(std::format_parse_context& ctx)
    { return ctx.begin(); }

    std::format_context::iterator format(T control, std::format_context& ctx) const;
};

template<typename T, coverpp::ColorControl auto... C>
struct std::formatter<coverpp::Styled<T, C...>> // NOLINT(*-dcl58-cpp)
{
    constexpr auto parse(std::format_parse_context& ctx)
    { return ctx.begin(); }

    std::format_context::iterator format(const coverpp::Styled<T, C...>& styled, std::format_context& ctx) const
    {
        auto out = ctx.out();
        ((out = std::format_to(out, "{}", C)), ...);
        return std::format_to(out, "{}{}", styled.payload, coverpp::Style::reset);
    }
};
