#pragma once

#include <format>
#include <functional>

namespace coverpp
{
struct VirtualAddress
{
    std::uintptr_t value;

    void* vp() const
    { return reinterpret_cast<void*>(value); }

    bool operator==(const VirtualAddress&) const = default;
};
}

template<>
struct std::formatter<coverpp::VirtualAddress>
{
    constexpr auto parse(std::format_parse_context& ctx)
    { return ctx.begin(); }

    auto format(coverpp::VirtualAddress ip, std::format_context& ctx) const
    {
        return std::format_to(ctx.out(), "0x{:X}", ip.value);
    }
};

template<>
struct std::hash<coverpp::VirtualAddress>
{
    std::size_t operator()(coverpp::VirtualAddress ip) const
    {
        return std::hash<decltype(ip.value)>()(ip.value);
    }
};
