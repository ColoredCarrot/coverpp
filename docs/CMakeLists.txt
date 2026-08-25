cmake_minimum_required(VERSION 3.28)
project(docs DESCRIPTION "Custom target to build docs" LANGUAGES NONE)

if (PROJECT_IS_TOP_LEVEL)
    include(CTest)
endif ()

# Discover source files
file(GLOB_RECURSE docs_sources
        RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
        CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.ts"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.tsx"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.md"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.mdx"
)

find_program(pnpm NAMES pnpm.exe pnpm.bat pnpm.cmd pnpm REQUIRED)

# Docs build step
add_custom_command(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/docs/index.html"
        # Build with --emptyOutDir to assure Vite that it's safe to clear the docs directory
        # even though it's not a subdirectory of the docs source directory.
        COMMAND "${pnpm}" build --outDir "${CMAKE_CURRENT_BINARY_DIR}/docs" --emptyOutDir
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        DEPENDS ${docs_sources}
)
add_custom_target(docs ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/docs/index.html")

if (BUILD_TESTING)
    # Register docs tests with CTest
    add_test(
            NAME docs-tests
            COMMAND "${pnpm}" test
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )
    set_tests_properties(docs-tests PROPERTIES TIMEOUT 60)
    message(STATUS "Will run docs tests in directory: ${CMAKE_CURRENT_SOURCE_DIR}")
endif ()
