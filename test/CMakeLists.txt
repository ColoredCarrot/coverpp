cmake_minimum_required(VERSION 3.27)
project(coverpp-test LANGUAGES CXX)

Include(FetchContent)

if (PROJECT_IS_TOP_LEVEL)
    include(CTest)
endif ()

FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.5.2
)

FetchContent_MakeAvailable(Catch2)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE
        "src/foo.test.cpp"
)

target_link_libraries(${PROJECT_NAME} PRIVATE Catch2::Catch2WithMain)

add_test(NAME unit-tests COMMAND ${PROJECT_NAME})
