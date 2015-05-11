#include <drivers/NavioRCIO_serial.h>

#include <cstring>
#include <errno.h>
#include <cstdio>
#include <unistd.h>
#include <fcntl.h>

#include <termio.h>
#include <err.h>
#include <linux/serial.h>

static int rate_to_constant(int baudrate) {
#define B(x) case x: return B##x
	switch(baudrate) {
		B(50);     B(75);     B(110);    B(134);    B(150);
		B(200);    B(300);    B(600);    B(1200);   B(1800);
		B(2400);   B(4800);   B(9600);   B(19200);  B(38400);
		B(57600);  B(115200); B(230400); B(460800); B(500000); 
		B(576000); B(921600); B(1000000);B(1152000);B(1500000); 
	default: return 0;
	}
#undef B
}    

/* Open serial port in raw mode, with custom baudrate if necessary */
int NavioRCIO_serial::_serial_open(const char *device, int rate)
{
	struct termios options;
	struct serial_struct serinfo;
	int fd;
	int speed = 0;

	/* Open and configure serial port */
	if ((fd = ::open(device,O_RDWR|O_NOCTTY)) == -1)
		return -1;

	speed = rate_to_constant(rate);

	if (speed == 0) {
		/* Custom divisor */
		serinfo.reserved_char[0] = 0;
		if (::ioctl(fd, TIOCGSERIAL, &serinfo) < 0)
			return -1;
		serinfo.flags &= ~ASYNC_SPD_MASK;
		serinfo.flags |= ASYNC_SPD_CUST;
		serinfo.custom_divisor = (serinfo.baud_base + (rate / 2)) / rate;
		if (serinfo.custom_divisor < 1) 
			serinfo.custom_divisor = 1;
		if (::ioctl(fd, TIOCSSERIAL, &serinfo) < 0)
			return -1;
		if (::ioctl(fd, TIOCGSERIAL, &serinfo) < 0)
			return -1;
		if (serinfo.custom_divisor * rate != serinfo.baud_base) {
			warnx("actual baudrate is %d / %d = %f",
			      serinfo.baud_base, serinfo.custom_divisor,
			      (float)serinfo.baud_base / serinfo.custom_divisor);
		}
	}

	::fcntl(fd, F_SETFL, 0);
	tcgetattr(fd, &options);
	cfsetispeed(&options, speed ?: B38400);
	cfsetospeed(&options, speed ?: B38400);
	cfmakeraw(&options);
	options.c_cflag |= (CLOCAL | CREAD);
	options.c_cflag &= ~CRTSCTS;
	if (tcsetattr(fd, TCSANOW, &options) != 0)
		return -1;

	return fd;
}

NavioRCIO_serial::NavioRCIO_serial():
    _baudrate(1500000)
{

    _fd = _serial_open("/dev/ttyUSB0", _baudrate);

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
	uint16_t *values = reinterpret_cast<uint16_t *>(data);

	if (count > PKT_MAX_REGS)
		return -EINVAL;


	int result;
	for (unsigned retries = 0; retries < 3; retries++) {

		_buffer.count_code = count | PKT_CODE_READ;
		_buffer.page = page;
		_buffer.offset = offset;

		/* start the transaction and wait for it to complete */
		result = _wait_complete();

		/* successful transaction? */
		if (result == OK) {

			/* check result in packet */
			if (PKT_CODE(_buffer) == PKT_CODE_ERROR) {

				/* IO didn't like it - no point retrying */
				result = -EINVAL;

			/* compare the received count with the expected count */
			} else if (PKT_COUNT(_buffer) != count) {

				/* IO returned the wrong number of registers - no point retrying */
				result = -EIO;

			/* successful read */				
			} else {

				/* copy back the result */
				memcpy(values, &_buffer.regs[0], (2 * count));
			}

			break;
		}
	}

	if (result == OK)
		result = count;
	return result;
}

int NavioRCIO_serial::write(unsigned address, const void *data, unsigned count)
{
    int ret;

    if (data == nullptr) {
        fprintf(stderr, "data null\n");
    }

    ret = ::write(_fd, data, count);

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

    ret = ::write(_fd, (char *) &_buffer,  PKT_SIZE(_buffer));

    if (ret < 0) {
        warn("write");
        return -EIO;
    }

    ret = ::read(_fd, (char *) &_buffer, PKT_SIZE(_buffer));

    if (ret < 0) {
        warn("read");
        return -EIO;
    } 

    return OK;
}

