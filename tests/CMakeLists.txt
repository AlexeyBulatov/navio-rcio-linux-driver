project(tests)

set(test1_source test1.cpp)

add_executable(test1 ${test1_source})
target_link_libraries(test1 libdrivers)
target_link_libraries(test1 gtest_main gtest)

add_test(NAME Test1 COMMAND test1)
