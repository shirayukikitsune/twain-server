cmake_minimum_required(VERSION 3.7)
project(dasa-twain-server)
enable_testing()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if (POLICY CMP0076)
  cmake_policy(SET CMP0076 NEW)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(Boost_USE_STATIC_LIBS        ON)  # only find static libs
set(Boost_USE_DEBUG_LIBS        OFF)  # ignore debug libs and
set(Boost_USE_RELEASE_LIBS       ON)  # only find release libs
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
find_package(Boost 1.71.0 REQUIRED COMPONENTS date_time filesystem system)

if (MSVC OR MSYS OR MINGW)
  set(APPLICATION_TYPE "Console" CACHE STRING "The generated application type: Console or Window")
else()
  find_package(FreeImage REQUIRED)
endif()

add_subdirectory(deps)
add_subdirectory(src)
add_subdirectory(test)
if (MSVC OR MSYS OR MINGW)
  add_subdirectory(watcher)
endif()
