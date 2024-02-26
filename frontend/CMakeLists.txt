cmake_minimum_required(VERSION 3.27)
project(frontend DESCRIPTION "Custom target to build frontend into BINARY_DIR/webapp" LANGUAGES NONE)

if (PROJECT_IS_TOP_LEVEL)
    include(CTest)
endif ()

add_custom_command(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/webapp/index.html"
        # Build with --emptyOutDir to assure Vite that it's safe to clear the webapp directory
        # even though it's not a subdirectory of the frontend source directory.
        COMMAND pnpm build --outDir "${CMAKE_CURRENT_BINARY_DIR}/webapp" --emptyOutDir
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)
add_custom_target(frontend ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/webapp/index.html")

#install(DIRECTORY) TODO

if (BUILD_TESTING)
    add_test(
            NAME frontend-tests
            COMMAND pnpm test
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )
endif ()
