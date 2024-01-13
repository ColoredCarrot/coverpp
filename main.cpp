#include <print>
#include <iostream>
#include <filesystem>

#define NOMINMAX

#include <wil/com.h>
#include <dia2.h>

int exec(std::convertible_to<std::string_view> auto &&... parts) {
    std::string s;
    (s.append(parts).append(" "), ...);
    return std::system(s.c_str());
}

void read_pdb(const std::filesystem::path &path) {
    wil::com_ptr<IDiaDataSource> com;

    if (const auto hr = CoCreateInstance(CLSID_DiaSource, nullptr, CLSCTX_INPROC_SERVER,
                                         __uuidof(IDiaDataSource), com.put_void()); FAILED(hr)) {
        std::println(std::cerr, "Could not CoCreate CLSID_DiaSource");
        return;
    }
}

int main() {
    read_pdb(
        R"(G:\Voidev\Official\Projects\C++\Cover++\cmake-build-debug-visual-studio\Debug\covercpp-work\Debug\example-sut.pdb)");
    return 0;

    std::string cmake = "cmake";

    std::filesystem::path project_dir = R"(G:\Voidev\Official\Projects\C++\Cover++\example-sut)";

    if (exec(cmake, "--version") != 0) {
        std::println(std::cerr, "CMake not found");
        return 1;
    }

    exec(cmake, "-S", project_dir.string(), "-B", "covercpp-work", "-G \"Visual Studio 17 2022\"");
    exec(cmake, "--build", "covercpp-work");

    // before and after every jump (conditional or not), insert a breakpoint instruction

    return 0;
}
