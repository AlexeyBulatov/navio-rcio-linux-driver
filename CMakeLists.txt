cmake_minimum_required(VERSION 2.8) 

project(navio-rcio-linux-driver CXX)

add_definitions("-Wall -std=c++11")

enable_testing()

find_package(Boost REQUIRED)

include_directories(include)

add_subdirectory(src)
add_subdirectory(tests)

add_executable(rcio rcio.cpp)
target_link_libraries(rcio libdrivers)
