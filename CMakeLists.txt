cmake_minimum_required(VERSION 3.22)
project(coverpp)

include(FetchContent)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

set(CMAKE_CXX_STANDARD 23)

add_executable(coverpp main.cpp)

# DIA SDK
find_package(DiaSDK REQUIRED)
target_include_directories(coverpp PUBLIC ${DIASDK_INCLUDE_DIRS})
target_link_libraries(coverpp ${DIASDK_LIBRARIES})

# WIL
set(WIL_BUILD_PACKAGING OFF)
set(WIL_BUILD_TESTS OFF)
FetchContent_Declare(
        wil
        GIT_REPOSITORY https://github.com/microsoft/wil.git
        GIT_TAG v1.0.231216.1
)

FetchContent_MakeAvailable(wil)

target_link_libraries(coverpp WIL)

add_subdirectory(example-sut)
