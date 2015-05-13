#include <cstdint>

#include <time.h>

#include <drivers/common.h>

static struct timespec _sketch_start_time;

static void common_time_init()
{
    clock_gettime(CLOCK_MONOTONIC, &_sketch_start_time);
}

void common_init()
{
    common_time_init();
}

uint64_t micros64()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return 1.0e6*((ts.tv_sec + (ts.tv_nsec*1.0e-9)) - 
                  (_sketch_start_time.tv_sec +
                   (_sketch_start_time.tv_nsec*1.0e-9)));

}

uint64_t get_elapsed_time(uint32_t *timestamp)
{
    uint64_t current_time = micros64();

    return (current_time - *timestamp);

}
