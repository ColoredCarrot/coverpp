#include "open_browser.hpp"

#define NOMINMAX

#include <Windows.h>
#include <conio.h>
#include <tchar.h>

namespace coverpp::detail
{
void open_browser_at_url(std::string_view url)
{
    ShellExecuteA(nullptr, nullptr, url.data(), nullptr, nullptr, SW_SHOWNORMAL);
}
}
