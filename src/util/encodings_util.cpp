#include "encodings_util.hpp"
#include "math_util.hpp"

#define NOMINMAX

#include <Windows.h>
#include <wil/result_macros.h>

namespace coverpp::windows
{
std::string utf16le_to_utf8(std::wstring_view utf16le)
{
    // This is not an optimization, but required, since WideCharToMultiByte returns 0 to indicate an error
    if (utf16le.empty())
    {
        return {};
    }

    const int utf16le_length{detail::convert_or_clamp<int>(utf16le.length())};

    const int num_bytes{WideCharToMultiByte(
        CP_UTF8, 0,
        utf16le.data(), utf16le_length,
        nullptr, 0,
        nullptr, nullptr
    )};
    THROW_LAST_ERROR_IF(num_bytes == 0);

    std::string utf8(num_bytes, '\0');
    THROW_LAST_ERROR_IF(!WideCharToMultiByte(
        CP_UTF8, 0,
        utf16le.data(), utf16le_length,
        utf8.data(), num_bytes,
        nullptr, nullptr
    ));

    return utf8;
}

std::string windows::utf16le_to_utf8(wchar_t utf16le)
{
    return utf16le_to_utf8(std::wstring_view{&utf16le, 1});
}

std::wstring utf8_to_utf16le(std::string_view utf8)
{
    // This is not an optimization, but required, since MultiByteToWideChar returns 0 to indicate an error
    if (utf8.empty())
    {
        return {};
    }

    const int utf8_length{detail::convert_or_clamp<int>(utf8.length())};

    const int utf16le_length{MultiByteToWideChar(
        CP_UTF8, 0,
        utf8.data(), utf8_length,
        nullptr, 0
    )};
    THROW_LAST_ERROR_IF(utf16le_length == 0);

    std::wstring utf16le(utf16le_length, '\0');
    THROW_LAST_ERROR_IF(!MultiByteToWideChar(
        CP_UTF8, 0,
        utf8.data(), utf8_length,
        utf16le.data(), utf16le_length
    ));

    return utf16le;
}
}
