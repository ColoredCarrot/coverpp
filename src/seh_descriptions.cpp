#include "seh_descriptions.hpp"

#include <format>

#define NOMINMAX

#include <Windows.h>

namespace coverpp
{
std::string_view describe_seh_exception_code(std::uint32_t code)
{
    switch (code)
    {
#define CASE(name) case name: return #name
    CASE(STILL_ACTIVE                      );
    CASE(EXCEPTION_ACCESS_VIOLATION        );
    CASE(EXCEPTION_DATATYPE_MISALIGNMENT   );
    CASE(EXCEPTION_BREAKPOINT              );
    CASE(EXCEPTION_SINGLE_STEP             );
    CASE(EXCEPTION_ARRAY_BOUNDS_EXCEEDED   );
    CASE(EXCEPTION_FLT_DENORMAL_OPERAND    );
    CASE(EXCEPTION_FLT_DIVIDE_BY_ZERO      );
    CASE(EXCEPTION_FLT_INEXACT_RESULT      );
    CASE(EXCEPTION_FLT_INVALID_OPERATION   );
    CASE(EXCEPTION_FLT_OVERFLOW            );
    CASE(EXCEPTION_FLT_STACK_CHECK         );
    CASE(EXCEPTION_FLT_UNDERFLOW           );
    CASE(EXCEPTION_INT_DIVIDE_BY_ZERO      );
    CASE(EXCEPTION_INT_OVERFLOW            );
    CASE(EXCEPTION_PRIV_INSTRUCTION        );
    CASE(EXCEPTION_IN_PAGE_ERROR           );
    CASE(EXCEPTION_ILLEGAL_INSTRUCTION     );
    CASE(EXCEPTION_NONCONTINUABLE_EXCEPTION);
    CASE(EXCEPTION_STACK_OVERFLOW          );
    CASE(EXCEPTION_INVALID_DISPOSITION     );
    CASE(EXCEPTION_GUARD_PAGE              );
    CASE(EXCEPTION_INVALID_HANDLE          );
    CASE(CONTROL_C_EXIT                    );
    default: return "<unknown exception>";
    }
}

static std::string describe_seh_exception_information(std::uint32_t code, std::span<const std::uintptr_t> information)
{
    // See https://learn.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-exception_record
    switch (code)
    {
    case EXCEPTION_ACCESS_VIOLATION:
    case EXCEPTION_IN_PAGE_ERROR:
    {
        auto base_description = std::format(
            "Invalid {} at address {}",
            information.empty()
            ? "unknown access" :
            information[0] == 0
            ? "read" :
            information[0] == 1
            ? "write" :
            information[0] == 8
            ? "data execution (DEP)"
            : "unknown access",
            information.size() < 2 ? "<unknown>" : std::format("0x{:X}", information[1])
        );
        if (code == EXCEPTION_IN_PAGE_ERROR)
        {
            return std::format(
                "{} with NTSTATUS {}",
                base_description,
                information.size() < 3 ? "<unknown>" : std::format("0x{:X}", information[2])
            );
        }
        return base_description;
    }
    default: return "No further information";
    }
}

std::string describe_seh_exception(std::uint32_t code, std::span<const std::uintptr_t> information)
{
    return std::format(
        "SEH exception 0x{:X} {}: {}",
        code,
        describe_seh_exception_code(code),
        describe_seh_exception_information(code, information)
    );
}
}
