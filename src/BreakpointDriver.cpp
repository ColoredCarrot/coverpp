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
	if (auto const existing = m_breakpoints.find(va); existing != m_breakpoints.end())
	{
		if (existing->second.installed)
		{
			// This happens often; multiple DIA line entries can point to the same VA
			return;
		}
		m_breakpoints.erase(existing);
	}

    std::byte original_instruction;
    THROW_LAST_ERROR_IF_NOT(ReadProcessMemory(m_process, va.vp(), &original_instruction, 1, nullptr));

    write_byte(va, std::byte{0xCC});

	m_breakpoints.emplace(va, Breakpoint{
		.original_instruction = original_instruction,
		.installed            = true,
	});
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
        throw std::logic_error{"Trying to remove breakpoint that does not exist"};
    }
	auto& breakpoint = it->second;

	if (!breakpoint.installed)
	{
		// This can happen in multithreaded applications:
		//  When WaitForDebugEventEx returns, there might be multiple debug (breakpoint) events in flight, coming from different threads.
		//  The first breakpoint event is handled, the original instruction is restored, and the instruction cache is flushed,
		//  BUT some other threads might have already executed the INT3.
		//  So, the next call to WaitForDebugEventEx will return immediately, with the same breakpoint event (just a different thread ID).
		//  Hence, this tombstone.
		return;
	}
	breakpoint.installed = false;

    write_byte(va, breakpoint.original_instruction);
}

void BreakpointDriver::write_byte(VirtualAddress va, std::byte byte)
{
    THROW_LAST_ERROR_IF_NOT(WriteProcessMemory(m_process, va.vp(), &byte, 1, nullptr));
    THROW_LAST_ERROR_IF_NOT(FlushInstructionCache(m_process, va.vp(), 1));
}
}
