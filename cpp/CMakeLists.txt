cmake_minimum_required(VERSION 3.17)

project(
  rumble
  VERSION 0.0.1
  LANGUAGES CXX)

# set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(CTest)
enable_testing()

include(getsubdirs)
subdirlist(SUBDIRS ${CMAKE_CURRENT_LIST_DIR})

foreach(subdir ${SUBDIRS})
  if(${subdir} MATCHES "build$")
    continue()
  endif()
  if(${subdir} MATCHES ".cache$")
    continue()
  endif()
  if(${subdir} MATCHES "cmake$")
    continue()
  endif()
  add_subdirectory(${subdir})
endforeach()
