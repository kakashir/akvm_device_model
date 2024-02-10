#ifndef __IOHUB_HPP
#define __IOHUB_HPP

#include "device.hpp"
#include <map>

typedef std::map<unsigned long, Idevice*> io_map;

class Iohub {
public:
	Iohub();
	~Iohub();
public:
	int register_port(unsigned long port, Idevice *i_device);
	void unregister_port(unsigned long port);

	int register_mmio(unsigned long mmio, Idevice *i_device);
	void unregister_mmio(unsigned long mmio);

	int dispatch_port_access(unsigned long type,
				 unsigned long size,
				 unsigned long &val);
	int dispatch_mmio_access(unsigned long type,
				 unsigned long size,
				 unsigned long &val);
private:
	int do_register(io_map &map, unsigned long index, Idevice *i_device);
	void do_unregister(io_map &map, unsigned long index);
private:
	io_map port_map;
	io_map mmio_map;
};

#endif
