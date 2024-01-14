#pragma once

#include <string>

#define NOMINMAX

#include <Windows.h>
#include <wil/result_macros.h>

#define THROW_LAST_ERROR_IF_NOT(x) THROW_LAST_ERROR_IF(!(x))

namespace coverpp::detail::windows
{
std::string bstr_to_utf8_string(BSTR bs);
}
