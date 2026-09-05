#pragma once

#include "types.hpp"

#include <unordered_map>
#include <format>

namespace coverpp
{
class BreakpointDriver
{
public:
    explicit BreakpointDriver(void* process);

    void set_breakpoint(VirtualAddress va);

	[[nodiscard]] bool has_breakpoint(VirtualAddress va) const;

    void remove_breakpoint(VirtualAddress va);

private:
    void* m_process;

    struct Breakpoint
    {
        std::byte original_instruction;
    	/**
    	 * Tombstone value; breakpoints are never actually removed from the map.
    	 * See comment in remove_breakpoint() for an explanation.
    	 */
    	bool installed;
    };

    std::unordered_map<VirtualAddress, Breakpoint> m_breakpoints;

    void write_byte(VirtualAddress va, std::byte byte);
};
}
