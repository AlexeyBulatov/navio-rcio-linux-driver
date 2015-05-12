cmake_minimum_required(VERSION 2.8) 

project(navio-rcio-linux-driver CXX)

add_definitions("-Wall -std=c++11")

include_directories(include)

add_subdirectory(src)

add_executable(rcio rcio.cpp)
target_link_libraries(rcio libdrivers)

if (test)
  # This adds another subdirectory, which has 'project(gtest)'.
  include_directories(lib/googletest/include)
  add_subdirectory(lib/googletest)
  add_subdirectory(tests)

  enable_testing()

  # Include the gtest library. gtest_SOURCE_DIR is available due to
  # 'project(gtest)' above.
  include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
endif()
