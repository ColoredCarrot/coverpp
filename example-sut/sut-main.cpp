int min(int a, int b) {
    if (a <= b) {
        __debugbreak();
        if (a == b) {
            return 99;
        }
        return a;
    }
    return b;
}

int main() {
    return min(1, 2);
}
