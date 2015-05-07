#ifndef _NAVIO_RCIO_H_
#define _NAVIO_RCIO_H_

#include "NavioRCIO_serial.h"

class NavioRCIO
{
    public:
        NavioRCIO();
        ~NavioRCIO();

       int detect(); 
       void poll();

    private:

        int _serial_open(char *dev, long baudrate);

        static const uint32_t  _io_reg_get_error = 0x80000000;

        int io_reg_get(uint8_t page, uint8_t offset, uint16_t *values, unsigned num_values);
        uint32_t io_reg_get(uint8_t page, uint8_t offset);

        NavioRCIO_serial *_interface;
        unsigned _max_transfer;

};
#endif
