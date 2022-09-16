cmake_minimum_required(VERSION 3.10)
project(thread_pool VERSION 1.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_EXPORT_COMPILE_COMMANDS true)

# set(CMAKE_BUILD_TYPE "Debug")

find_package(benchmark REQUIRED)
find_package(Boost REQUIRED container)
find_package(absl REQUIRED flat_hash_map)
add_executable(main main.cpp)

target_link_libraries(main PRIVATE benchmark)
target_link_libraries(main PRIVATE Boost::container)
target_link_libraries(main PRIVATE absl::flat_hash_map)
