#pragma once

#include <string>

namespace coverpp::windows
{
/**
 * Convert a string from UTF-16 LE (the native Windows unicode encoding) to UTF-8.
 */
std::string utf16le_to_utf8(std::wstring_view utf16le);
std::string utf16le_to_utf8(wchar_t utf16le);

std::wstring utf8_to_utf16le(std::string_view utf8);
}
