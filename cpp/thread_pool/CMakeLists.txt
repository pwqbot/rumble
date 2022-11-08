find_package(spdlog REQUIRED)

set(LINK_LIBRARIES spdlog::spdlog)

# include(catchtest)
catchtest(${LINK_LIBRARIES})
