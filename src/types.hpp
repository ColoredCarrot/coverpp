#pragma once

#include <format>
#include <functional>

namespace coverpp
{
struct VirtualAddress
{
    std::uintptr_t value;
};

struct InstructionPointer
{
    std::uintptr_t value;

    void* vp() const
    { return reinterpret_cast<void*>(value); }

    bool operator==(const InstructionPointer&) const = default;
};
}

template<>
struct std::formatter<coverpp::VirtualAddress>
{
    constexpr auto parse(std::format_parse_context& ctx)
    { return ctx.begin(); }

    auto format(coverpp::VirtualAddress va, std::format_context& ctx) const
    {
        return std::format_to(ctx.out(), "0x{:X}", va.value);
    }
};

template<>
struct std::formatter<coverpp::InstructionPointer>
{
    constexpr auto parse(std::format_parse_context& ctx)
    { return ctx.begin(); }

    auto format(coverpp::InstructionPointer ip, std::format_context& ctx) const
    {
        return std::format_to(ctx.out(), "0x{:X}", ip.value);
    }
};

template<>
struct std::hash<coverpp::InstructionPointer>
{
    std::size_t operator()(coverpp::InstructionPointer ip) const
    {
        return std::hash<decltype(ip.value)>()(ip.value);
    }
};
