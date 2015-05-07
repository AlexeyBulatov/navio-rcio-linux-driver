#include <cstdio>

#include <modules/px4iofirmware/protocol.h>
#include <drivers/NavioRCIO.h>

int main(int argc, char *argv[])
{
    NavioRCIO io{};

    // io.detect();

    while (true) {
        io.poll();
    }

    return 0;
}
