cmake_minimum_required(VERSION 3.27)
project(coverpp)

cmake_policy(SET CMP0091 NEW)

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/MP>")

#set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

include(FetchContent)
include(cmake/CMakeRC.cmake)
include(cmake/CopyAsanDLLs.cmake)

set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)
include(InstallRequiredSystemLibraries)

cmrc_add_resource_library(coverpp-resources ALIAS coverpp::rc NAMESPACE coverpp_rc
    "src/resources/html-exporter/index.html"
    "src/resources/html-exporter/index.css"
    "src/resources/html-exporter/third_party/prism.js"
    "src/resources/html-exporter/third_party/prism.css"
)

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
        src/exporter/SourceFileExporter.cpp
        src/exporter/SourceFileExporter.hpp
        src/exporter/Exporter.hpp
        src/report/CoverageProcessor.cpp
        src/report/CoverageProcessor.hpp
        src/report/BasicReport.cpp
        src/report/BasicReport.hpp
        src/exporter/html/HtmlExporter.cpp
        src/exporter/html/HtmlExporter.hpp
        src/seh_descriptions.hpp
        src/seh_descriptions.cpp
        src/util/encodings_util.cpp
        src/util/encodings_util.hpp
        src/util/math_util.hpp
        src/exporter/raw/RawExporter.cpp
        src/exporter/raw/RawExporter.hpp
        src/exporter/AbstractFileExporter.cpp
        src/exporter/AbstractFileExporter.hpp
        src/exporter/teamcity/TeamCityStatisticsExporter.cpp
        src/exporter/teamcity/TeamCityStatisticsExporter.hpp
        src/serve/ServeOptions.hpp
        src/serve/serve.cpp
        src/serve/serve.hpp
        src/util/open_browser.cpp
        src/util/open_browser.hpp
        src/util/console_color.cpp
        src/util/console_color.hpp
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

#add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>" "$<$<C_COMPILER_ID:MSVC>:/fsanitize=address>")
#add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>" "$<$<CXX_COMPILER_ID:MSVC>:/fsanitize=address>")

#set_property(TARGET coverpp PROPERTY
#        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

target_compile_options(coverpp PUBLIC "$<$<C_COMPILER_ID:MSVC>:/utf-8>" "$<$<C_COMPILER_ID:MSVC>:/Zc:char8_t->")
target_compile_definitions(coverpp PRIVATE "_WIN32_WINNT=0x0A00") # Windows 10 and above

# DIA SDK
find_package(DiaSDK REQUIRED)
target_include_directories(coverpp PUBLIC
        ${DIASDK_INCLUDE_DIRS}
        "${CMAKE_SOURCE_DIR}/third_party/CLI11/include"
        "${CMAKE_SOURCE_DIR}/third_party/asio/include"
        "${CMAKE_SOURCE_DIR}/third_party/Crow/include"
)
target_link_libraries(coverpp PRIVATE ${DIASDK_LIBRARIES} coverpp::rc)

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
target_link_libraries(coverpp PRIVATE WIL)

# Copy DIA SDK DLL to same directory as .exe
# See https://stackoverflow.com/questions/10671916/how-to-copy-dll-files-into-the-same-folder-as-the-executable-using-cmake
add_custom_command(TARGET coverpp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${DIASDK_DLL}" $<TARGET_FILE_DIR:coverpp>
)

copy_asan_dlls(coverpp)


set_target_properties(coverpp PROPERTIES OUTPUT_NAME "cover++")

install(TARGETS coverpp)


add_subdirectory(example-sut)
