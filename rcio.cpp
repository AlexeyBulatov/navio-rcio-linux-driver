#include <cstdio>

#include <modules/px4iofirmware/protocol.h>
#include <drivers/NavioRCIO.h>

int main(int argc, char *argv[])
{
    NavioRCIO io{};

    printf("Hi, there\n");
    return 0;
}
