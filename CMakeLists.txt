cmake_minimum_required(VERSION 3.7)
project(gliese-scanner)
enable_testing()

cmake_policy(SET CMP0076 NEW)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Boost REQUIRED filesystem)

add_subdirectory(deps)
add_subdirectory(src)
