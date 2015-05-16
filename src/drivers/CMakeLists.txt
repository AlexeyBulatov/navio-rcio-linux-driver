project(libdrivers CXX)

set(libdrivers_src
    NavioRCIO.cpp
    NavioRCIO_serial.cpp
    common.cpp
)

add_library(libdrivers ${libdrivers_src})
target_link_libraries(libdrivers rt)
