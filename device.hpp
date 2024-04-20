#ifndef __DEVICE_H
#define __DEVICE_H

#include "common.hpp"

#define DEVICE_IO_IN
#define DEVICE_IO_OUT

interface Idevice {
	virtual ~Idevice() {;}
	virtual int handle_port_write(gpa addr, u64 size, u64 val) = 0;
	virtual int handle_port_read(gpa addr, u64 size, u64 &val) = 0;
	virtual int handle_mmio_write(gpa addr, u64 size, u64 val) = 0;
	virtual int handle_mmio_read(gpa addr, u64 size, u64 &val) = 0;

	virtual gpa start_addr(void) const = 0;
	virtual gpa end_addr(void) const = 0;
};

#endif
