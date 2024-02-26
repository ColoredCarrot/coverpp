cmake_minimum_required(VERSION 3.27)
project(frontend DESCRIPTION "Custom target to build frontend into BINARY_DIR/webapp" LANGUAGES NONE)

include(FetchContent)

if (PROJECT_IS_TOP_LEVEL)
    include(CTest)
endif ()

set(FLATBUFFERS_BUILD_TESTS OFF)
FetchContent_Declare(
        flatbuffers
        GIT_REPOSITORY https://github.com/google/flatbuffers.git
        GIT_TAG v23.5.26
)

FetchContent_MakeAvailable(flatbuffers)

# Code generation
set(FLATBUFFERS_FLATC_SCHEMA_EXTRA_ARGS "--ts" "--gen-object-api")
build_flatbuffers(
        #[[flatbuffers_schemas]]     "${CMAKE_CURRENT_SOURCE_DIR}/../schemas/coverage_report.fbs"
        #[[schema_include_dirs]]     ""
        #[[custom_target_name]]      coverpp-schemas-frontend
        #[[additional_dependencies]] ""
        #[[generated_includes_dir]]  "${CMAKE_CURRENT_SOURCE_DIR}/src/_generated"
        #[[binary_schemas_dir]]      ""
        #[[copy_text_schemas_dir]]   ""
)

# Frontend build step
add_custom_command(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/webapp/index.html"
        # Build with --emptyOutDir to assure Vite that it's safe to clear the webapp directory
        # even though it's not a subdirectory of the frontend source directory.
        COMMAND pnpm build --outDir "${CMAKE_CURRENT_BINARY_DIR}/webapp" --emptyOutDir
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        DEPENDS coverpp-schemas-frontend
)
add_custom_target(frontend ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/webapp/index.html")

#install(DIRECTORY) TODO

if (BUILD_TESTING)
    # Register frontend tests with CTest
    add_test(
            NAME frontend-tests
            COMMAND pnpm test
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )
    set_tests_properties(frontend-tests PROPERTIES TIMEOUT 60)
    message(STATUS "Will run frontend tests in directory: ${CMAKE_CURRENT_SOURCE_DIR}")
endif ()
