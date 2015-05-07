#include <drivers/NavioRCIO_serial.h>

#include <cstring>
#include <errno.h>
#include <cstdio>
#include <unistd.h>
#include <fcntl.h>

NavioRCIO_serial::NavioRCIO_serial()
{
    _fd = open("/dev/ttyUSB0", O_RDWR);

    if (_fd < 0) {
        fprintf(stderr, "ERROR\n");
    }

}

NavioRCIO_serial::~NavioRCIO_serial()
{

}

int NavioRCIO_serial::init()
{
    return 0;
}

int NavioRCIO_serial::read(unsigned address, void *data, unsigned count)
{
    uint8_t page = address >> 8;
    uint8_t offset = address & 0xff;
    const uint16_t *values = reinterpret_cast<const uint16_t *>(data);

    if (count > PKT_MAX_REGS)
        return -EINVAL;

    int result;

    for (unsigned retries = 0; retries < 3; retries++) {
        _buffer.count_code = count | PKT_CODE_WRITE;
        _buffer.page = page;
        _buffer.offset = offset;
        memcpy((void *)&_buffer.regs[0], (void *)values, (2 * count));
        for (unsigned i = count; i < PKT_MAX_REGS; i++)
            _buffer.regs[i] = 0x55aa;
        /* XXX implement check byte */
        /* start the transaction and wait for it to complete */
        result = _wait_complete();
        /* successful transaction? */
        if (result == OK) {
            /* check result in packet */
            if (PKT_CODE(_buffer) == PKT_CODE_ERROR) {
                /* IO didn't like it - no point retrying */
                result = -EINVAL;
            }
            break;
        }
    }

    if (result == OK)
        result = count;

    return result;
}

int NavioRCIO_serial::write(unsigned address, void *data, unsigned count)
{
    int ret;

    ret = write(_fd, data, count);

    return ret;
}

int NavioRCIO_serial::ioctl(unsigned operation, unsigned &arg)
{
    return 0;
}


int NavioRCIO_serial::_wait_complete()
{
    int ret;

    _buffer.crc = 0;
    _buffer.crc = crc_packet(&_buffer);

    
    ret = write(_fd, (char *) &_buffer,  PKT_SIZE(_buffer));

    if (ret < 0) {
        fprintf(stderr, "write error\n");
    }

    return OK;
}

