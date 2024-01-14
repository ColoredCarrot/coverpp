#include "BreakpointDriver.hpp"

#include <cassert>
#include <print>

#define NOMINMAX

#include <wil/com.h>
#include <dia2.h>
#include <psapi.h>
#include <intrin.h>

#define THROW_LAST_ERROR_IF_NOT(x) THROW_LAST_ERROR_IF(!(x))

namespace coverpp
{
BreakpointDriver::BreakpointDriver(void* process) : m_process{process}
{}

void BreakpointDriver::set_base_address(InstructionPointer base_address)
{
    m_base_address = base_address;
}

VirtualAddress BreakpointDriver::ip_to_va(InstructionPointer ip) const
{
    // VA = IP - base
    return VirtualAddress{ip.value - m_base_address.value};
}
InstructionPointer BreakpointDriver::va_to_ip(VirtualAddress va) const
{
    // IP = VA + base
    return InstructionPointer{va.value + m_base_address.value};
}

void BreakpointDriver::set_breakpoint(InstructionPointer ip)
{
    std::println("set {:x} base {:x}", ip.value, m_base_address.value);

    remove_breakpoint(ip);

    std::byte original_instruction;
    THROW_LAST_ERROR_IF_NOT(ReadProcessMemory(m_process, ip.vp(), &original_instruction, 1, nullptr));

    write_byte(ip, std::byte{0xCC});

    m_breakpoints.emplace(ip, Breakpoint{original_instruction});
}

void BreakpointDriver::remove_breakpoint(InstructionPointer ip)
{
    const auto it = m_breakpoints.find(ip);
    if (it == m_breakpoints.end()) {
        return;
    }

    write_byte(ip, it->second.original_instruction);

    m_breakpoints.erase(it);
}

void BreakpointDriver::write_byte(InstructionPointer ip, std::byte byte)
{
    THROW_LAST_ERROR_IF_NOT(WriteProcessMemory(m_process, ip.vp(), &byte, 1, nullptr));
    THROW_LAST_ERROR_IF_NOT(FlushInstructionCache(m_process, ip.vp(), 1));
}
}
