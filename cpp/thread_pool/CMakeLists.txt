cmake_minimum_required(VERSION 3.10)
project(thread_pool VERSION 1.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_EXPORT_COMPILE_COMMANDS true)
# set(CMAKE_BUILD_TYPE Debug)

include(FetchContent)


FetchContent_Declare(
    gtest
    GIT_REPOSITORY https://github.com/google/googletest
    GIT_TAG main
    )
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog
    GIT_TAG v1.x
    )


FetchContent_MakeAvailable(gtest)
FetchContent_MakeAvailable(spdlog)

enable_testing()
add_executable(mytest test_pool.cpp)
# target_compile_options(mytest PRIVATE -fsanitize=thread)
# target_link_options(mytest PRIVATE -fsanitize=thread)
target_link_libraries(mytest PRIVATE GTest::gtest_main)
target_link_libraries(mytest PRIVATE spdlog)

include(GoogleTest)
gtest_discover_tests(mytest)
