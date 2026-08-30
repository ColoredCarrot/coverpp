#include "com_utils.hpp"

namespace coverpp::windows::detail
{
std::wstring_view bstr_to_wstring_view(BSTR bs)
{
	if (!bs)
	{
		return {};
	}
	return std::wstring_view{bs, SysStringLen(bs)};
}

std::string bstr_to_utf8_string(BSTR bs)
{
    // See https://stackoverflow.com/questions/6284524/bstr-to-stdstring-stdwstring-and-vice-versa

    const std::size_t num_wchars{SysStringLen(bs)};

    // This is not an optimization, but required, since WideCharToMultiByte returns 0 to indicate an error
    if (num_wchars == 0) {
        return {};
    }

    //TODO check if num_wchars > max int
    const int num_bytes{WideCharToMultiByte(
        CP_UTF8, 0, bs, static_cast<int>(num_wchars), nullptr, 0, nullptr, nullptr
    )};
    THROW_LAST_ERROR_IF(num_bytes == 0);

    std::string utf8(num_bytes, '\0');
    THROW_LAST_ERROR_IF_NOT(WideCharToMultiByte(
        CP_UTF8, 0, bs, static_cast<int>(num_wchars), utf8.data(), num_bytes, nullptr, nullptr
    ));

    return utf8;
}
}
