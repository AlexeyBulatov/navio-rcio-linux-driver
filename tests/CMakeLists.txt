project(tests)

find_package (Boost COMPONENTS unit_test_framework REQUIRED)

set(test1_source test1.cpp)
add_executable(test1 ${test1_source})
target_link_libraries(test1 ${Boost_LIBRARIES} libdrivers)

add_test(NAME Test1 COMMAND test1)
