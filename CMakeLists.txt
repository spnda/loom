cmake_minimum_required(VERSION 3.9)

project("loom")

set(CMAKE_CXX_STANDARD 17)

add_executable(loom)

add_subdirectory(${CMAKE_SOURCE_DIR}/src)
