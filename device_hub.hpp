#ifndef _DEVICE_HUB_HPP
#define _DEVCEE_HUB_HPP

#include "common.hpp"
#include <list>

class Idevice;
class DeviceHub {
public:
	static DeviceHub& instance(void)
	{
		static DeviceHub *obj;

		if (!obj)
			obj = new DeviceHub;
		return *obj;
	}

	int register_device(Idevice *device);
	void unregister_device(Idevice *device);

	int handle_port_write(gpa addr, u64 size, u64 val);
	int handle_port_read(gpa addr, u64 size,u64 &val);
	int handle_mmio_write(gpa addr, u64 size, u64 val);
	int handle_mmio_read(gpa addr, u64 size, u64 &val);
private:
	DeviceHub(void);
	~DeviceHub(void);
	void sort_devices_by_addr(void);
	bool device_addr_range_overlap(const Idevice *l,
				       const Idevice *r);
	Idevice* find_device_by_addr(gpa addr);
	bool incorrect_size(u64 size);

	std::list<Idevice*> m_devices;
};

#endif
