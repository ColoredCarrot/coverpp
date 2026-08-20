#pragma once

#include <span>
#include <string>

namespace coverpp
{
std::string_view describe_seh_exception_code(std::uint32_t code);

std::string describe_seh_exception(std::uint32_t code, std::span<const std::uintptr_t> information);
}
