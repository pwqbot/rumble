add_library(range range.cpp)

find_package(range-v3 REQUIRED)
find_package(fmt)
target_link_libraries(range range-v3::range-v3)
target_link_libraries(range fmt::fmt)

set(LINK_LIBRARIES range-v3::range-v3 fmt::fmt)

message("${LINK_LIBRARIES}")
include(catchtest)
catchtest(${LINK_LIBRARIES})
