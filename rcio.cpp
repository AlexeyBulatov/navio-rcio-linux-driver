#include <cstdio>
#include <cstdlib>

#include <unistd.h>

#include <modules/px4iofirmware/protocol.h>
#include <drivers/NavioRCIO.h>
#include <drivers/NavioRCInput.h>
#include <drivers/NavioRCOutput.h>

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
        fprintf(stderr, "bus interface not initialized\n");
        return 1;
    }

    NavioRCIO io{&interface};

    if (!io.init()) {
        fprintf(stderr, "Init failed\n");
        return 1;
    }

    if (io.detect()) {
        fprintf(stderr, "Detected\n");

        io.ioctl(PX4IO_SET_DEBUG, 5);

        struct pwm_output_rc_config config;

        config.channel       =  0x1;
        config.rc_min        =  900;
        config.rc_trim       =  1600;
        config.rc_max        =  2100;
        config.rc_dz         =  100;
        config.rc_assignment =  0x1;
        config.rc_reverse    =  0x0;

        io.ioctl(PWM_SERVO_SET_RC_CONFIG, (unsigned long) &config);

        bool initialized;

        io.ioctl(IO_GET_INIT_STATUS, (unsigned long) &initialized);

        if (!initialized) {
            fprintf(stderr, "init failed\n");
        }

        io.ioctl(PWM_SERVO_SET_FORCE_SAFETY_OFF, 0);
        io.ioctl(PWM_SERVO_SET_ARM_OK, 0);
        io.ioctl(PWM_SERVO_ARM, 0);

        while(true) {
            int ret;
            
            unsigned int pulse = 1700;

            ret = io.ioctl(PWM_SERVO_SET(1), (unsigned long) pulse);

            if (ret < 0) {
                fprintf(stderr, "failed to write\n");
            } else {
                io.print_status(true);
            }
        }
    } else {
        fprintf(stderr, "Not detected\n");
    }

    return 0;
}
