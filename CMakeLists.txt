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

option(EnableImageConversion "Enable conversion and compression of image types" ON)

find_package(Boost 1.71.0 REQUIRED date_time filesystem system)
if (EnableImageConversion)
  find_package(ImageMagick REQUIRED COMPONENTS Magick++)
endif()

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
