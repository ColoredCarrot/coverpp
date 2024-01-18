#define __STDC_WANT_SECURE_LIB__ 1

#include "sut-foo.hpp"

#include <ranges>
#include <string_view>
#include <format>

#define NOMINMAX
#include <windows.h>

int min(int a, int b) {
    if (a <= b) {
//        __debugbreak();
        if (a == b) {
            return 99;
        }
        return a;
    }
    return b;
}

void seh() {
    __try {
        int p = 1 + 2;
        RaiseException(1, 0, 0, nullptr);
        ++p;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        int q = 1 + 2;
    }
}

int main(int argc, char** argv) {
    const std::span<char*> args{argv, static_cast<std::size_t>(argc)};

    auto args_str = args
                    | std::views::transform([](char* arg) { return std::string_view{arg}; })
                    | std::views::join_with(std::string_view{", "})
                    | std::ranges::to<std::string>();

    OutputDebugString(std::format("{} args: {}", argc, args_str).c_str());

    for (int i : std::views::iota(0, 10))
    {
        add(1, i);
    }

    seh();

    return min(1, 2);
}

namespace foo {
    int main() {
        return 42;
    }
}
