#pragma once

#include <string>

#define NOMINMAX

#include <Windows.h>
#include <wil/com.h>
#include <wil/result_macros.h>

#define THROW_LAST_ERROR_IF_NOT(x) THROW_LAST_ERROR_IF(!(x))

namespace coverpp::detail
{
}

namespace coverpp::windows::detail
{
using namespace coverpp::detail;

std::wstring_view bstr_to_wstring_view(BSTR bs);

std::string bstr_to_utf8_string(BSTR bs);

template<typename T>
std::string get_string(const wil::com_ptr<T>& com, HRESULT (T::* f)(BSTR*))
{
    BSTR bs;
    THROW_IF_FAILED(((*com).*f)(&bs));
    return bstr_to_utf8_string(bs);
}

template<std::integral V, typename T>
V get_dword(const wil::com_ptr<T>& com, HRESULT (T::* f)(V*))
{
    V v;
    THROW_IF_FAILED(((*com).*f)(&v));
    return v;
}

template<std::integral V, typename T>
V get_dword_r(T& com, HRESULT (T::* f)(V*))
{
    V v;
    THROW_IF_FAILED((com.*f)(&v));
    return v;
}

template<typename TItem, typename TEnum>
wil::com_ptr<TItem> get_single_item(TEnum& enumeration)
{
    LONG count;
    THROW_IF_FAILED(enumeration.get_Count(&count));
    if (count != 1)
    {
        return {};
    }

    wil::com_ptr<TItem> item;
    THROW_IF_FAILED(enumeration.Item(0, item.put()));
    return item;
}
}
