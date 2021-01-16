cmake_minimum_required(VERSION 3.0)
project(workshop1)

set(CMAKE_CXX_STANDARD 17)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(main main.cpp automata.cpp)
add_executable(tests tests.cpp automata.cpp)

target_link_libraries(tests gtest gtest_main pthread)

enable_testing()
add_test(NAME tests COMMAND tests)

add_custom_target(testing
        COMMAND echo ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND mkdir test_dir && cd test_dir
        COMMAND g++-7 -std=c++17 --coverage -pthread ../automata.cpp ../tests.cpp -lgtest -lgtest_main -lpthread -o test
        COMMAND ./test
        COMMAND lcov -t "test" -o test.info --capture --directory . --gcov-tool /usr/bin/gcov-7
        COMMAND lcov --remove test.info "/usr/include/*" "/usr/local/*" "*googletest/*" "/usr/include/gtest" "/usr/include/gtest/internal" "/7/*" -o test.info
        COMMAND genhtml -o coverage_report test.info
)