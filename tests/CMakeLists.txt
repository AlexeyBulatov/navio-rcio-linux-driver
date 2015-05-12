project(tests)

set(rcio_src rcio_test.cpp)

add_executable(rcio_test ${rcio_src})
target_link_libraries(rcio_test libdrivers gtest_main gtest)

add_test(NAME RCIO_Test COMMAND rcio_test)
