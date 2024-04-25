#ifndef _DEVICE_SERIAL_HPP
#define _DEVICE_SERIAL_HPP

#include "common.hpp"
#include "device.hpp"
#include "platform.h"

class DeviceSerial : public Idevice
{
public:
	DeviceSerial(void);
	virtual ~DeviceSerial(void);
public:
	int handle_port_write(gpa addr, u64 size, u64 val);
	int handle_port_read(gpa addr, u64 size, u64 &val);
	int handle_mmio_write(gpa addr, u64 size, u64 val);
	int handle_mmio_read(gpa addr, u64 size, u64 &val);

	gpa start_addr(void) const
	{ return DEVICE_SERIAL_PORT_MMIO_BEGIN; }

	gpa end_addr(void) const
	{ return DEVICE_SERIAL_PORT_MMIO_END; }
};

#endif
