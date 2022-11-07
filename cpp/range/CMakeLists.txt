add_library(range range.cpp)

find_package(range-v3 REQUIRED)
find_package(fmt)
target_link_libraries(range range-v3::range-v3)
target_link_libraries(range fmt::fmt)
