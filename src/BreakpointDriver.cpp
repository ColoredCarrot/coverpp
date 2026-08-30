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

void BreakpointDriver::set_breakpoint(VirtualAddress va)
{
    remove_breakpoint(va);

    std::byte original_instruction;
    THROW_LAST_ERROR_IF_NOT(ReadProcessMemory(m_process, va.vp(), &original_instruction, 1, nullptr));

    write_byte(va, std::byte{0xCC});

    m_breakpoints.emplace(va, Breakpoint{original_instruction});
}

bool BreakpointDriver::has_breakpoint(VirtualAddress va) const
{
	return m_breakpoints.contains(va);
}

void BreakpointDriver::remove_breakpoint(VirtualAddress va)
{
    const auto it = m_breakpoints.find(va);
    if (it == m_breakpoints.end())
    {
        return;
    }

    write_byte(va, it->second.original_instruction);

    m_breakpoints.erase(it);
}

void BreakpointDriver::write_byte(VirtualAddress va, std::byte byte)
{
    THROW_LAST_ERROR_IF_NOT(WriteProcessMemory(m_process, va.vp(), &byte, 1, nullptr));
    THROW_LAST_ERROR_IF_NOT(FlushInstructionCache(m_process, va.vp(), 1));
}
}
