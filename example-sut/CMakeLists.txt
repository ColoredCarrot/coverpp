cmake_minimum_required(VERSION 3.22)
project(example-sut)

set(CMAKE_CXX_STANDARD 23)

include(../cmake/CopyAsanDLLs.cmake)

add_executable(example-sut sut-main.cpp sut-foo.hpp
        deep/deep.hpp
        deep/nested/a.hpp
        deep/nested/b.hpp)

target_compile_options(example-sut PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/Wall /WX>
)
target_link_options(example-sut PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/WX>
)

copy_asan_dlls(example-sut)
