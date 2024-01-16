#pragma once

#include <algorithm>

namespace coverpp::detail
{
template<std::integral R, std::integral V>
constexpr R convert_or_clamp(V value)
{
    if (std::cmp_less(value, std::numeric_limits<R>::min()))
    {
        return std::numeric_limits<R>::min();
    }
    if (std::cmp_greater(value, std::numeric_limits<R>::max()))
    {
        return std::numeric_limits<R>::max();
    }
    return static_cast<R>(value);
}

static_assert(convert_or_clamp<int>(std::numeric_limits<long>::max()) == std::numeric_limits<int>::max());
}
