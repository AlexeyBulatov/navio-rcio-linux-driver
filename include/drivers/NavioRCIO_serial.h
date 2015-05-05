#ifndef _NAVIO_RCIO_SERIAL_H_
#define _NAVIO_RCIO_SERIAL_H_

#include "modules/px4iofirmware/protocol.h"

class NavioRCIO_serial
{
    public:
        NavioRCIO_serial();
        virtual ~NavioRCIO_serial();
        virtual int init();
        virtual int read(unsigned offset, void *data, unsigned count = 1);
        virtual int write(unsigned address, void *data, unsigned count = 1);
        virtual int ioctl(unsigned operation, unsigned &arg);
    private:
        IOPacket    _buffer;
        int         _fd;
};

#endif
