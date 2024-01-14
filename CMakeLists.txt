cmake_minimum_required(VERSION 3.27)
project(coverpp)

cmake_policy(SET CMP0091 NEW)

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/MP>")

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

include(FetchContent)
include(InstallRequiredSystemLibraries)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

set(CMAKE_CXX_STANDARD 23)

add_executable(coverpp
    "main.cpp"
    "src/BreakpointDriver.hpp"
    "src/BreakpointDriver.cpp"
        src/CoverageSink.cpp
        src/CoverageSink.hpp
        src/types.hpp
        src/com_utils.cpp
        src/com_utils.hpp
        src/CoverageEngine.hpp
        src/WindowsCoverageSession.cpp
        src/WindowsCoverageSession.hpp
        src/CoverageParams.hpp
        src/DiaAccessor.cpp
        src/DiaAccessor.hpp
        src/file_util.cpp
        src/file_util.hpp
        src/report/SourceFileReportGenerator.cpp
        src/report/SourceFileReportGenerator.hpp
        src/report/ReportGenerator.hpp
        src/report/CoverageProcessor.cpp
        src/report/CoverageProcessor.hpp
        src/report/BasicReport.cpp
        src/report/BasicReport.hpp
)
# TODO Once CMake 3.28 is supported:
#[[add_executable(coverpp)

target_sources(coverpp
    PUBLIC
        FILE_SET CXX_MODULES FILES
            "src/DebugEngine.cppm"
        FILE_SET HEADERS FILES
)
target_sources(coverpp
    PRIVATE
        "main.cpp"
        "src/DebugEngine.cpp"
)]]

# TODO copy clang_rt.asan_dynamic-x86_64.dll just like the other dll
# D:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.38.33130\bin\Hostx86\x64
# D:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.38.33130\bin\Hostx64\x64
target_compile_options(coverpp PUBLIC "$<$<C_COMPILER_ID:MSVC>:/utf-8>" "$<$<C_COMPILER_ID:MSVC>:/fsanitize=address>")
target_compile_options(coverpp PUBLIC "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>" "$<$<CXX_COMPILER_ID:MSVC>:/fsanitize=address>")
set_property(TARGET coverpp PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

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

#$<$<CONFIG:DEBUG>:-wholearchive:clang_rt.asan_dbg-x86_64.lib$<SEMICOLON>-NODEFAULTLIB:LIBCMTD>
target_link_libraries(coverpp WIL)

# Copy DIA SDK DLL to same directory as .exe
# See https://stackoverflow.com/questions/10671916/how-to-copy-dll-files-into-the-same-folder-as-the-executable-using-cmake
add_custom_command(TARGET coverpp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${DIASDK_DLL}" $<TARGET_FILE_DIR:coverpp>
)

set(MSVC_DLL_DIR [=[D:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.38.33130\bin\Hostx64\x64\]=])
set(MSVC_DLL_LIST
    "${MSVC_DLL_DIR}clang_rt.asan_dynamic-x86_64.dll"
    "${MSVC_DLL_DIR}clang_rt.asan_dbg_dynamic-x86_64.dll"
)
foreach (dll ${MSVC_DLL_LIST})
    add_custom_command(TARGET coverpp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${dll}" $<TARGET_FILE_DIR:coverpp>
    )
endforeach ()

add_subdirectory(example-sut)
