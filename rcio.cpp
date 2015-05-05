#include <cstdio>

#include <modules/px4iofirmware/protocol.h>
#include <drivers/NavioRCIO.h>

int main(int argc, char *argv[])
{
    NavioRCIO io{};

    io.detect();

    return 0;
}
