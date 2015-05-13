#include <cstdio>
#include <cstdlib>

#include <unistd.h>

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

    NavioRCIO_serial interface{};

    if (!interface.init()) {
        fprintf(stderr, "bus interface not initialized");
        return 1;
    }

    NavioRCIO io{&interface};

    if (!io.init()) {
        fprintf(stderr, "Init failed\n");
        return 1;
    }

    if (io.detect()) {
        fprintf(stderr, "Detected\n");

        /* simple write test */
        if (argc > 1) 
            io.ioctl(PX4IO_SET_DEBUG, atoi(argv[1]));

        while(true) {
            io.print_status(true);
            sleep(1);
        }
    } else {
        fprintf(stderr, "Not detected\n");
    }

    return 0;
}
