#include <cstdio>
#include <cstdlib>

#include <modules/px4iofirmware/protocol.h>
#include <drivers/NavioRCIO.h>

char *progname;
static void display_usage() 
{
    fprintf(stderr, "usage: %s debug_level\n", progname);
}

int main(int argc, char *argv[])
{
    progname = argv[0];

    NavioRCIO io{};

    if (io.detect()) {
        fprintf(stderr, "Detected\n");

        /* simple write test */
        if (argc > 1) 
            io.ioctl(PX4IO_SET_DEBUG, atoi(argv[1]));

        io.print_status(true);
    } else {
        fprintf(stderr, "Not detected\n");
    }

    return 0;
}
