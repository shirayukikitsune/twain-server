cmake_minimum_required(VERSION 3.7)
project(dasa-twain-server)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if (POLICY CMP0076)
  cmake_policy(SET CMP0076 NEW)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Boost 1.71.0 REQUIRED date_time filesystem system)

if (MSVC OR MSYS OR MINGW)
  set(APPLICATION_TYPE "Console" CACHE STRING "The generated application type: Console, Window or Service")
else()
  find_package(FreeImage REQUIRED)
endif()

add_subdirectory(deps)
add_subdirectory(src)
add_subdirectory(test)
