project(libdrivers CXX)

set(libdrivers_src
    NavioRCIO.cpp
    NavioRCIO_serial.cpp
)

add_library(libdrivers ${libdrivers_src})
target_link_libraries(libdrivers)
