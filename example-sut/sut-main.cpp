//#define NOMINMAX
//#include <windows.h>

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
    return min(1, 2);
}

namespace foo {
    int main() {
        return 42;
    }
}
