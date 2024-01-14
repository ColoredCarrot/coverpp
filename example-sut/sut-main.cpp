//#define NOMINMAX
//#include <windows.h>
#include "sut-foo.hpp"

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
    return min(1, add(1, 1));
}

namespace foo {
    int main() {
        return 42;
    }
}
