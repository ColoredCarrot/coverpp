module;

#include <unordered_map>

export module coverpp.DebugEngine;

export namespace coverpp
{
struct VirtualAddress
{
    std::intptr_t value;
};
}

export template<>
struct std::hash<coverpp::VirtualAddress>
{
    std::size_t operator()(coverpp::VirtualAddress va) const
    {
        return std::hash<decltype(va.value)>()(va.value);
    }
};

export namespace coverpp
{
class DebugEngine
{
public:
    void set_breakpoint(VirtualAddress address);

    void remove_breakpoint(VirtualAddress address);

private:
    struct Breakpoint
    {
        std::byte original_instruction;
    };

    std::unordered_map<VirtualAddress, Breakpoint> m_breakpoints;
};


void DebugEngine::set_breakpoint(VirtualAddress address)
{
    remove_breakpoint(address);



//    m_breakpoints.emplace(address, Breakpoint{.original_instruction = });
}
void DebugEngine::remove_breakpoint(VirtualAddress address)
{

}
}
