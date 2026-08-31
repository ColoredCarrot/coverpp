#include "seh_descriptions.hpp"

#include "windows/exception_codes.hpp"
#include "windows/util.hpp"

#include <format>

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

    // From https://learn.microsoft.com/en-us/windows-hardware/drivers/debugger/specific-exceptions
	CASE(STATUS_ASSERTION_FAILURE);
	CASE(STATUS_APPLICATION_HANG);
	CASE(STATUS_CPP_EH_EXCEPTION);
	CASE(STATUS_CLR_EXCEPTION);
	CASE(DBG_CONTROL_BREAK);
	CASE(DBG_CONTROL_C);
	CASE(DBG_COMMAND_EXCEPTION);
	CASE(STATUS_STACK_BUFFER_OVERRUN);
	CASE(STATUS_VERIFIER_STOP);

    CASE(VS_SET_THREAD_NAME);

    default: return "<unknown exception>";
    }
}

static std::string describe_seh_exception_information(std::uint32_t code, std::span<const std::uintptr_t> information, void* process, std::uint32_t thread_id)
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
	case VS_SET_THREAD_NAME: {
		if (information.size() >= 4 && information[0] == 0x1000)
		{
			auto name_ptr = reinterpret_cast<char const*>(information[1]);
			auto name     = windows::detail::read_remote_c_string(process, name_ptr);

			auto target_thread_id = static_cast<DWORD>(information[2]);
			if (target_thread_id == static_cast<DWORD>(-1))
			{
				target_thread_id = thread_id;
			}

			auto flags = static_cast<DWORD>(information[3]);

			return std::format("Set name of thread {} to {}{}",
			                   thread_id,
			                   name,
			                   flags ? std::format(" (but dwFlags is 0x{:X} when it should be 0!)", flags) : "");
		}
		break;
	}
    case STATUS_CPP_EH_EXCEPTION: {

    }
    // TODO: We could decode STATUS_CPP_EH_EXCEPTION
    // See https://devblogs.microsoft.com/oldnewthing/20100730-00/?p=13273
    // See https://github.com/microsoft/STL/blob/main/stl/src/excptptr.cpp
    default: break;
    }

	return "No further information";
}

std::string describe_seh_exception(std::uint32_t code, std::span<const std::uintptr_t> information, void* process, std::uint32_t thread_id)
{
    return std::format(
        "SEH exception 0x{:X} {}: {}",
        code,
        describe_seh_exception_code(code),
        describe_seh_exception_information(code, information, process, thread_id)
    );
}
}
