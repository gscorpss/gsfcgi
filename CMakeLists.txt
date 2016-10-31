cmake_minimum_required(VERSION 2.6)
project(gsfcgi)

add_definitions(-std=c++11)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

enable_testing(true)

add_subdirectory(src)
add_subdirectory(test)
