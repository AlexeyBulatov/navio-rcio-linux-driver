#ifndef _NAVIO_RCIO_SERIAL_H_
#define _NAVIO_RCIO_SERIAL_H_

#include "modules/px4iofirmware/protocol.h"

#define OK 1

class NavioRCIO_serial
{
    public:
        NavioRCIO_serial();
        virtual ~NavioRCIO_serial();
        virtual bool init();
        virtual int read(unsigned offset, void *data, unsigned count = 1);
        virtual int write(unsigned address, const void *data, unsigned count = 1);
        virtual int ioctl(unsigned operation, unsigned &arg);
    private:
        int _wait_complete();
        int _serial_open(const char *device, int rate);
        
        IOPacket    _buffer;
        int         _fd;
        long        _baudrate;
};

#endif
