cmake_minimum_required(VERSION 3.17)
project(practicum2)

set(CMAKE_CXX_STANDARD 17)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(main main.cpp earley.cpp)
add_executable(tests tests.cpp earley.cpp)

target_link_libraries(tests ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} Threads::Threads)

enable_testing()
add_test(NAME tests COMMAND tests)

add_custom_target(testing
        COMMAND echo ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND mkdir test_dir && cd test_dir
        COMMAND echo ${CMAKE_CXX_STANDARD}
        COMMAND g++-7 -std=c++${CMAKE_CXX_STANDARD} --coverage -pthread ../earley.cpp ../tests.cpp ${GTEST_LIBRARIES} -o test
        COMMAND ./test
        COMMAND lcov -t "test" -o test.info --capture --directory . --gcov-tool /usr/bin/gcov-7
        COMMAND lcov --remove test.info "/usr/include/*"
                "/usr/local/*" "*googletest/*" "/usr/include/gtest/"
                "*/gtest/*" "/usr/include/c++/7/*"
                "/usr/include/c++/7/*" "*/bits/*" "*/ext/*" -o test.info
        COMMAND genhtml -o coverage_report test.info
        )