#include <errno.h>
#include <unistd.h>
#include <cstdio>
#include "device_serial.hpp"

DeviceSerial::DeviceSerial(void)
{;}

DeviceSerial::~DeviceSerial(void)
{;}

int DeviceSerial::handle_port_write(gpa addr, u64 size, u64 val)
{
	return -ENOTSUP;
}

int DeviceSerial::handle_port_read(gpa addr, u64 size, u64 &val)
{
	return -ENOTSUP;
}

int DeviceSerial::handle_mmio_write(gpa addr, u64 size, u64 val)
{
	if (addr == DEVICE_SERIAL_MMIO_DATA_OUT) {
		write(STDOUT_FILENO, &val, size);
		return 0;
	}
	return -ENOTSUP;
}

int DeviceSerial::handle_mmio_read(gpa addr, u64 size, u64 &val)
{
	return -ENOTSUP;
}
