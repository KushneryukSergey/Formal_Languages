cmake_minimum_required(VERSION 3.0)
project(automata)

set(CMAKE_CXX_STANDARD 17)
add_definitions(-fprofile-arcs -ftest-coverage)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -g -O0 -Wall -fprofile-arcs -ftest-coverage")
set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE ON)

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(automata main/main.cpp)
add_executable(TESTS tests/tests.cpp)


set(OBJECT_DIR ${CMAKE_BINARY_DIR}/CMakeFiles/TESTS.dir)

add_custom_target(gcov
        COMMAND mkdir -p gcoverage
        COMMAND ${CMAKE_MAKE_PROGRAM}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
add_custom_command(TARGET gcov
        COMMAND echo "=================== GCOV ===================="
        COMMAND echo "-- Source directory: ${CMAKE_SOURCE_DIR}"
        COMMAND gcov -b ${CMAKE_SOURCE_DIR}/*.cpp -o ${OBJECT_DIR}
        COMMAND echo "-- Coverage files have been output to ${CMAKE_BINARY_DIR}/gcoverage"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/gcoverage
    )
add_dependencies(gcov TESTS)

add_custom_target(lcov
    COMMAND mkdir -p lcoverage
    )
add_custom_command(TARGET lcov
    COMMAND echo "=================== LCOV ===================="
    COMMAND echo "-- Passing lcov tool under code coverage"
    COMMAND lcov --capture --directory . --output-file lcoverage/main_coverage.info
    COMMAND echo "-- Generating HTML output files"
    COMMAND genhtml lcoverage/main_coverage.info --output-directory lcoverage
    )

set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES gcoverage)

add_custom_target(init
    COMMAND ${CMAKE_MAKE_PROGRAM} clean
    COMMAND rm -f ${OBJECT_DIR}/*.gcno
    COMMAND rm -f ${OBJECT_DIR}/*.gcda
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )


target_link_libraries(TESTS gtest gtest_main pthread)
enable_testing()
add_test(NAME TESTS COMMAND TESTS)