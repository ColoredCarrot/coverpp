cmake_minimum_required(VERSION 3.22)
project(example-sut)

set(CMAKE_CXX_STANDARD 23)

add_compile_options(/FAs)

add_executable(example-sut sut-main.cpp)

target_compile_options(example-sut PUBLIC /FAs)
