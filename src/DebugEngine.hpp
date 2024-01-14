#include <unordered_map>
#include <format>

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
        return std::format_to(ctx.out(), "{:x}", va.value);
    }
};

template<>
struct std::formatter<coverpp::InstructionPointer>
{
    constexpr auto parse(std::format_parse_context& ctx)
    { return ctx.begin(); }

    auto format(coverpp::InstructionPointer ip, std::format_context& ctx) const
    {
        return std::format_to(ctx.out(), "{:x}", ip.value);
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

namespace coverpp
{
class DebugEngine
{
public:
    explicit DebugEngine(void* process);

    void set_base_address(InstructionPointer base_address);

    VirtualAddress ip_to_va(InstructionPointer ip) const;
    InstructionPointer va_to_ip(VirtualAddress va) const;

    void set_breakpoint(InstructionPointer ip);

    void remove_breakpoint(InstructionPointer ip);

private:
    void* m_process;
    InstructionPointer m_base_address;

    struct Breakpoint
    {
        std::byte original_instruction;
    };

    std::unordered_map<InstructionPointer, Breakpoint> m_breakpoints;

    void write_byte(InstructionPointer ip, std::byte byte);
};
}
