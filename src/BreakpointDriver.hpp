#include "types.hpp"

#include <unordered_map>
#include <format>

namespace coverpp
{
class BreakpointDriver
{
public:
    explicit BreakpointDriver(void* process);

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
