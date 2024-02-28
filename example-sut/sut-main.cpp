#include "sut-foo.hpp"
#include "ony/one/child.hpp"
#include "deep/deep.hpp"
#include "deep/nested/a.hpp"
#include "deep/nested/b.hpp"

#include <format>
#include <ranges>
#include <string_view>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 5039) // winbase.h(7810,48): warning C5039: 'TpSetCallbackCleanupGroup': pointer or reference to potentially throwing function passed to 'extern "C"' function under -EHc. Undefined behavior may occur if this function throws an exception.
#endif

#include <Windows.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

int min(int a, int b)
{
    if (a <= b)
    {
//        __debugbreak();
        if (a == b)
        {
            return 99;
        }
        return a;
    }
    return b;
}

void seh()
{
    __try
    {
        int p = 1 + 2;
        RaiseException(1, 0, 0, nullptr);
        ++p;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        [[maybe_unused]] int q = 1 + 2;
    }
}

int main(int argc, char** argv)
{
    const std::span<char*> args{argv, static_cast<std::size_t>(argc)};

    auto args_str = args
                    | std::views::transform([](char* arg) { return std::string_view{arg}; })
                    | std::views::join_with(std::string_view{", "})
                    | std::ranges::to<std::string>();

    OutputDebugString(std::format("{} args: {}", argc, args_str).c_str());

    only_one_child();
    deep();
    a();
    b();

    for (int i : std::views::iota(0, 10))
    {
        add(1, i);
    }

    seh();

    return min(1, 2);
}

namespace foo
{
int main()
{
    return 42;
}
}
