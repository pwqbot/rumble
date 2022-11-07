cmake_minimum_required(VERSION 3.17)

project(
  rumble
  VERSION 0.0.1
  LANGUAGES CXX)

# set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

macro(SUBDIRLIST result curdir)
  file(
    GLOB children
    RELATIVE ${curdir}
    ${curdir}/*)
  set(dirlist "")
  foreach(child ${children})
    if(IS_DIRECTORY ${curdir}/${child})
      list(APPEND dirlist ${child})
    endif()
  endforeach()
  set(${result} ${dirlist})
endmacro()

subdirlist(SUBDIRS ${CMAKE_CURRENT_LIST_DIR})

foreach(subdir ${SUBDIRS})
  if(${subdir} MATCHES "build$")
    continue()
  endif()
  if(${subdir} MATCHES ".cache$")
    continue()
  endif()
  add_subdirectory(${subdir})
endforeach()

include(CTest)
enable_testing()
