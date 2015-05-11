#include <errno.h>
#include <cstdio>
#include <unistd.h>

#include <drivers/NavioRCIO.h>

#define debug(...) fprintf(stderr, __VA_ARGS__)
#define log(...) debug(__VA_ARGS__)

NavioRCIO::NavioRCIO():
    _interface(nullptr),
    _max_transfer(16)
{
    _interface = new NavioRCIO_serial();

    if (_interface == nullptr) {
        fprintf(stderr, "_interface == nullptr\n");
    }
}

NavioRCIO::~NavioRCIO()
{
    if (_interface != nullptr) {
        delete _interface;
    }
}

void NavioRCIO::poll()
{
    static const size_t ALPHABET_SIZE = 50;

    static int i = 0;

    char *buffer = new char[i];

    for (int j = 0; j < i; j++) {
        buffer[j] = 'a' + j;
    }

    _interface->write(0x5a, buffer, i);

    i = (i + 1) % ALPHABET_SIZE;

    delete [] buffer;
}

bool NavioRCIO::detect()
{
     unsigned protocol = io_reg_get(PX4IO_PAGE_CONFIG, PX4IO_P_CONFIG_PROTOCOL_VERSION);

     if (protocol != PX4IO_PROTOCOL_VERSION) {
         fprintf(stderr, "protocol: 0x%x\n", protocol);
         if (protocol == _io_reg_get_error) {
             log("IO not installed\n");
         } else {
             log("IO version error\n");
         }
         return false;
     }
     return true;
}


int NavioRCIO::io_reg_get(uint8_t page, uint8_t offset, uint16_t *values, unsigned num_values)
{
    /* range check the transfer */
    if (num_values > ((_max_transfer) / sizeof(*values))) {
        debug("io_reg_get: too many registers (%u, max %u)", num_values, _max_transfer / 2);
        return -EINVAL;
    }
    int ret = _interface->read((page << 8) | offset, reinterpret_cast<void *>(values), num_values);
    if (ret != (int)num_values) {
        debug("io_reg_get(%u,%u,%u): data error %d", page, offset, num_values, ret);
        return -1;
    }
    return OK;

}

uint32_t NavioRCIO::io_reg_get(uint8_t page, uint8_t offset)
{
    uint16_t value;
    if (io_reg_get(page, offset, &value, 1) != OK)
        return _io_reg_get_error;
    return value;
}
