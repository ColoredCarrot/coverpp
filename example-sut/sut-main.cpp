//#define NOMINMAX
//#include <windows.h>
#include "sut-foo.hpp"

#include <ranges>

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

int main() {
//    OutputDebugString("Hello, world!");

    for (int i : std::views::iota(0, 10))
    {
        add(1, i);
    }

    return min(1, 2);
}

namespace foo {
    int main() {
        return 42;
    }
}
