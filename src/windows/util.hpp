#pragma once

#include <string>

namespace coverpp::detail
{
}

namespace coverpp::windows::detail
{
using namespace coverpp::detail;

/**
 * Read a null-terminated string via ReadProcessMemory.
 *
 * The read is performed in chunks for performance.
 */
template<typename TChar = char>
    requires std::same_as<TChar, char> || std::same_as<TChar, wchar_t>
std::basic_string<TChar> read_remote_c_string(void* process, std::uintptr_t address, std::size_t max_length = 1024);

template<typename TChar = char>
    requires std::same_as<TChar, char> || std::same_as<TChar, wchar_t>
std::basic_string<TChar> read_remote_c_string(void* process, TChar const* address, std::size_t max_length = 1024);

std::wstring read_remote_c_wstring(void* process, std::uintptr_t address, std::size_t max_length = 1024);
std::wstring read_remote_c_wstring(void* process, wchar_t const* address, std::size_t max_length = 1024);

} // namespace coverpp::windows::detail
