cmake_minimum_required(VERSION 3.7)
project(dasa-twain-server)

# cmake_policy(SET CMP0076 NEW)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Boost 1.71.0 REQUIRED filesystem system)

add_subdirectory(deps)
add_subdirectory(src)
