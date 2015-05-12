#ifndef _NAVIO_RCIO_H_
#define _NAVIO_RCIO_H_

#include "NavioRCIO_serial.h"

#define PX4IO_SET_DEBUG 1

class NavioRCIO
{
    public:
        NavioRCIO();
        ~NavioRCIO();

        bool init();
       bool detect(); 
       void poll();
       int ioctl(int cmd, unsigned long arg);
       void print_status(bool extended_status);

    private:

        int _serial_open(char *dev, long baudrate);

        static const uint32_t  _io_reg_get_error = 0x80000000;

        int io_reg_get(uint8_t page, uint8_t offset, uint16_t *values, unsigned num_values);
        uint32_t io_reg_get(uint8_t page, uint8_t offset);
        int io_reg_set(uint8_t page, uint8_t offset, const uint16_t *values, unsigned num_values);
        int io_reg_set(uint8_t page, uint8_t offset, uint16_t value);
        int io_reg_modify(uint8_t page, uint8_t offset, uint16_t clearbits, uint16_t setbits);

        NavioRCIO_serial *_interface;
        unsigned		_hardware;		///< Hardware revision
	    unsigned		_max_actuators;		///< Maximum # of actuators supported by PX4IO
	    unsigned		_max_controls;		///< Maximum # of controls supported by PX4IO
	    unsigned		_max_rc_input;		///< Maximum receiver channels supported by PX4IO
	    unsigned		_max_relays;		///< Maximum relays supported by PX4IO
	    unsigned		_max_transfer;		///< Maximum number of I2C transfers supported by PX4IO
        float			_battery_amp_per_volt;	///< current sensor amps/volt
	    float			_battery_amp_bias;	///< current sensor bias
	    float			_battery_mamphour_total;///< amp hours consumed so far
	    uint64_t		_battery_last_timestamp;///< last amp hour calculation timestamp

};
#endif
