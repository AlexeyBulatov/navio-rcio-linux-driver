#ifndef _NAVIO_RCIO_H_
#define _NAVIO_RCIO_H_

#include "NavioRCIO_serial.h"

class NavioRCIO
{
    public:
        NavioRCIO();
        ~NavioRCIO();

    private:
        NavioRCIO_serial *interface;

};
#endif
