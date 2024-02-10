#ifndef __DEVICE_H
#define __DEVICE_H

#include "common.hpp"

interface Idevice {
	virtual ~Idevice() = 0;
	virtual int handle_port_io(unsigned long type,
				   unsigned long size,
				   unsigned long &val) = 0;
	virtual int handle_mmio(unsigned long type,
				unsigned long size,
				unsigned long &val) = 0;

	/*
	  interface to expose the "detail" to hw_cfg interface is
	  not easy to define... let's think it more before add the
	  definition
	 */
};

#endif
