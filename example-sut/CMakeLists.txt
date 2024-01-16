cmake_minimum_required(VERSION 3.22)
project(example-sut)

set(CMAKE_CXX_STANDARD 23)

include(../cmake/CopyAsanDLLs.cmake)

add_executable(example-sut sut-main.cpp sut-foo.hpp)

copy_asan_dlls(example-sut)
