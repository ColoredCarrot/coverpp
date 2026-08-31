cmake_minimum_required(VERSION 3.28)
project(coverpp VERSION 26.0.2)

cmake_policy(SET CMP0091 NEW)

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/MP>")

#set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

include(FetchContent)
include(cmake/CMakeRC.cmake)
include(cmake/CopyAsanDLLs.cmake)

if (PROJECT_IS_TOP_LEVEL)
    include(CTest)
endif ()

include(GNUInstallDirs)

set(CMAKE_INSTALL_UCRT_LIBRARIES OFF) # Not needed on Windows 10+
set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION "${CMAKE_INSTALL_BINDIR}")
set(CMAKE_INSTALL_SYSTEM_RUNTIME_COMPONENT c1_Runtime)
include(InstallRequiredSystemLibraries)

# Write a file containing the version, so that
file(GENERATE OUTPUT "${CMAKE_BINARY_DIR}/version.txt" CONTENT "${PROJECT_VERSION}")

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
        src/CoverageEngine.hpp
        src/CoverageParams.hpp
        src/CoverageSink.cpp
        src/CoverageSink.hpp
        src/DiaAccessor.cpp
        src/DiaAccessor.hpp
        src/WindowsCoverageSession.cpp
        src/WindowsCoverageSession.hpp
        src/com_utils.cpp
        src/com_utils.hpp
        src/docs/docs.cpp
        src/docs/docs.hpp
        src/exporter/Exporter.hpp
        src/exporter/clion/CLionExporter.cpp
        src/exporter/clion/CLionExporter.hpp
        src/exporter/clion/clion_export_options.hpp
        src/exporter/export_options.hpp
        src/exporter/html/HtmlExporter.cpp
        src/exporter/html/HtmlExporter.hpp
        src/exporter/html/html_export_options.hpp
        src/exporter/json/JsonExporter.cpp
        src/exporter/json/JsonExporter.hpp
        src/exporter/json/json_export_options.hpp
        src/exporter/raw/RawExporter.cpp
        src/exporter/raw/RawExporter.hpp
        src/exporter/teamcity/TeamCityStatisticsExporter.cpp
        src/exporter/teamcity/TeamCityStatisticsExporter.hpp
        src/file_util.cpp
        src/file_util.hpp
        src/merge/merge_reports.cpp
        src/merge/merge_reports.hpp
        src/merge/MergeOptions.hpp
        src/remap/remap.cpp
        src/remap/remap.hpp
        src/report/BasicReport.cpp
        src/report/BasicReport.hpp
        src/report/CoverageProcessor.cpp
        src/report/CoverageProcessor.hpp
        src/report/flat_report.cpp
        src/report/flat_report.hpp
        src/report/report_file_utils.cpp
        src/report/report_file_utils.hpp
        src/seh_descriptions.cpp
        src/seh_descriptions.hpp
        src/serve/ServeOptions.hpp
        src/serve/serve.cpp
        src/serve/serve.hpp
        src/stats/calculate_stats.cpp
        src/stats/calculate_stats.hpp
        src/types.hpp
        src/util/console_color.cpp
        src/util/console_color.hpp
        src/util/crow_util.cpp
        src/util/crow_util.hpp
        src/util/encodings_util.cpp
        src/util/encodings_util.hpp
        src/util/guard.hpp
        src/util/math_util.hpp
        src/util/open_browser.cpp
        src/util/open_browser.hpp
        src/util/progress_bar.hpp
        src/util/progress_bar.cpp
        src/windows/exception_codes.hpp
        src/windows/util.cpp
        src/windows/util.hpp
        src/polyfill/generator.hpp
        src/run.cpp
        src/run.hpp
)
# TODO Once CMake 3.28 is supported (CLion 2024.1):
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

# Enable warnings
target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W2 /WX>
)
target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/WX>
)

# Disable char8_t
target_compile_options(${PROJECT_NAME} PUBLIC
        $<$<CXX_COMPILER_ID:MSVC>:/Zc:char8_t->
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-fno-char8_t>
)

# Enable UTF-8
target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<C_COMPILER_ID:MSVC>:/utf-8>
)

# Windows 10 and above
target_compile_definitions(${PROJECT_NAME} PRIVATE "_WIN32_WINNT=0x0A00")

# Pass app version
target_compile_definitions(${PROJECT_NAME} PRIVATE
        "COVERPP_VERSION=${PROJECT_VERSION}"
        "COVERPP_VERSION_STRING=\"${PROJECT_VERSION}\""
)

# We don't use C++20 modules
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_SCAN_FOR_MODULES OFF)

# DIA SDK
find_package(DiaSDK REQUIRED)
message(STATUS "Found DIA SDK: ${DIASDK_DLL}")
target_include_directories(coverpp PUBLIC
        ${DIASDK_INCLUDE_DIRS}
        "${CMAKE_SOURCE_DIR}/third_party/asio/include"
        "${CMAKE_SOURCE_DIR}/third_party/CLI11/include"
        "${CMAKE_SOURCE_DIR}/third_party/Crow/include"
        "${CMAKE_SOURCE_DIR}/third_party/gch_small_vector/include"
        "${CMAKE_SOURCE_DIR}/third_party/rapidjson/include"
        "${CMAKE_SOURCE_DIR}/third_party/softwareverify_exceptionCodes/include"
)
target_link_libraries(coverpp PRIVATE ${DIASDK_LIBRARIES} coverpp::rc)

# WIL
set(WIL_BUILD_PACKAGING OFF)
set(WIL_BUILD_TESTS OFF)
FetchContent_Declare(
        wil
        GIT_REPOSITORY https://github.com/microsoft/wil.git
        GIT_TAG v1.0.231216.1
        EXCLUDE_FROM_ALL
)

# FlatBuffers
set(FLATBUFFERS_BUILD_TESTS OFF)
set(FLATBUFFERS_INSTALL OFF)
FetchContent_Declare(
        flatbuffers
        GIT_REPOSITORY https://github.com/google/flatbuffers.git
        GIT_TAG v23.5.26
)

FetchContent_MakeAvailable(wil flatbuffers)

#$<$<CONFIG:DEBUG>:-wholearchive:clang_rt.asan_dbg-x86_64.lib$<SEMICOLON>-NODEFAULTLIB:LIBCMTD>

set(FLATBUFFERS_FLATC_SCHEMA_EXTRA_ARGS "--gen-object-api")
build_flatbuffers(
        #[[flatbuffers_schemas]]     "${CMAKE_CURRENT_SOURCE_DIR}/schemas/coverage_report.fbs"
        #[[schema_include_dirs]]     ""
        #[[custom_target_name]]      coverpp-schemas
        #[[additional_dependencies]] ""
        #[[generated_includes_dir]]  "${CMAKE_CURRENT_SOURCE_DIR}/_generated/include"
        #[[binary_schemas_dir]]      ""
        #[[copy_text_schemas_dir]]   ""
)

add_dependencies(coverpp coverpp-schemas)

target_link_libraries(coverpp PRIVATE WIL flatbuffers)

# Copy DIA SDK DLL to same directory as .exe
# See https://stackoverflow.com/questions/10671916/how-to-copy-dll-files-into-the-same-folder-as-the-executable-using-cmake
add_custom_command(TARGET coverpp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${DIASDK_DLL}" $<TARGET_FILE_DIR:coverpp>
)

# Depend on frontend
add_subdirectory(frontend)
add_dependencies(coverpp frontend)

# Depend on docs
add_subdirectory(docs)
add_dependencies(coverpp docs)

copy_asan_dlls(coverpp)

# To be able to run cover++.exe without installing it (most likely during development),
# copy the webapp directory from the frontend target's BINARY_DIR to the main executable's BINARY_DIR
# (so right next to the .exe).
get_target_property(frontend_binary_dir frontend BINARY_DIR)
add_custom_command(TARGET coverpp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${frontend_binary_dir}/webapp" "$<TARGET_FILE_DIR:coverpp>/webapp"
)

get_target_property(docs_binary_dir docs BINARY_DIR)


set_target_properties(coverpp PROPERTIES OUTPUT_NAME "cover++")


install(
        TARGETS coverpp
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
        COMPONENT c1_Runtime
)
install(
        FILES "${DIASDK_DLL}"
        DESTINATION "${CMAKE_INSTALL_BINDIR}"
        COMPONENT c1_Runtime
)
install(
        DIRECTORY "${frontend_binary_dir}/webapp/"
        DESTINATION "${CMAKE_INSTALL_BINDIR}/webapp"
        COMPONENT c1_Runtime
)
install(
        DIRECTORY "${docs_binary_dir}/docs/"
        DESTINATION "docs"
        COMPONENT c2_Documentation
)


add_subdirectory(example-sut)


if (BUILD_TESTING)
    add_subdirectory(test)
endif ()


##########################################################################
#  Packaging

set(CPACK_PACKAGE_NAME "Cover++")
set(CPACK_PACKAGE_VENDOR "Julian K.")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "C++ code coverage tool for Windows")

set(CPACK_PACKAGE_INSTALL_DIRECTORY "Cover++")

set(CPACK_GENERATOR "ZIP" "INNOSETUP")

# Inno Setup settings
set(CPACK_INNOSETUP_ARCHITECTURE "x64")
set(CPACK_INNOSETUP_USE_MODERN_WIZARD ON)

# Don't show CPack's dummy license page (we don't have a license ourselves)
set(CPACK_INNOSETUP_IGNORE_LICENSE_PAGE ON)

# By default, Inno Setup doesn't let you choose a different directory if the app is already installed
set(CPACK_INNOSETUP_SETUP_DisableDirPage OFF)

# Add a task to add cover++ to PATH
set(CPACK_INNOSETUP_SETUP_ChangesEnvironment ON)
set(CPACK_INNOSETUP_EXTRA_SCRIPTS "${CMAKE_CURRENT_SOURCE_DIR}/inno-setup/modpath-task.iss")
set(CPACK_INNOSETUP_CODE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/inno-setup/modpath-code.iss")

include(CPackComponent)

cpack_add_install_type(full DISPLAY_NAME "Full")

# Note: We use these internal names with the c1_ prefix so that we can control their order
# (the order of cpack_add_component invocations does not matter)
cpack_add_component(c1_Runtime REQUIRED DISPLAY_NAME "Runtime")
cpack_add_component(c2_Documentation DISPLAY_NAME "Documentation" INSTALL_TYPES full)

include(CPack)
