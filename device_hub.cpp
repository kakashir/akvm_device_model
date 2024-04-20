#include <errno.h>
#include "device.hpp"
#include "device_hub.hpp"

DeviceHub::DeviceHub()
{;}

DeviceHub::~DeviceHub()
{
	/*
	  Device hub doesn't own the device ptr
	  do just clear the list without free each Device*
	 */
	m_devices.clear();
}

int DeviceHub::register_device(Idevice *device)
{
	if (!device)
		return -EINVAL;

	for (auto i : m_devices) {
		if (device_addr_range_overlap(i, device))
			return -EINVAL;
		if (device == i)
			return -EEXIST;
	}

	m_devices.push_back(device);
	sort_devices_by_addr();
	return 0;
}

void DeviceHub::unregister_device(Idevice *device)
{
	if (!device)
		return;

	if (m_devices.empty())
		return;

	for (auto it = m_devices.begin(); it != m_devices.end(); ++it) {
		if (*it != device)
			continue;

		m_devices.erase(it);
		return;
	}
}

int DeviceHub::handle_port_write(gpa addr, u64 size, u64 val)
{
	Idevice *device;

	if (incorrect_size(size))
		return -EINVAL;

	device = find_device_by_addr(addr);
	if (!device)
		return -ENOTSUP;

	return device->handle_port_write(addr, size, val);
}

int DeviceHub::handle_port_read(gpa addr, u64 size, u64 &val)
{
	Idevice *device;

	if (incorrect_size(size))
		return -EINVAL;

	device = find_device_by_addr(addr);
	if (!device)
		return -ENOTSUP;

	return device->handle_port_read(addr, size, val);
}

int DeviceHub::handle_mmio_write(gpa addr, u64 size, u64 val)
{
	Idevice *device;

	if (incorrect_size(size))
		return -EINVAL;

	device = find_device_by_addr(addr);
	if (!device)
		return -ENOTSUP;

	return device->handle_mmio_write(addr, size, val);
}

int DeviceHub::handle_mmio_read(gpa addr, u64 size, u64 &val)
{
	Idevice *device;

	if (incorrect_size(size))
		return -EINVAL;

	device = find_device_by_addr(addr);
	if (!device)
		return -ENOTSUP;

	return device->handle_mmio_read(addr, size, val);
}

void DeviceHub::sort_devices_by_addr(void)
{
	m_devices.sort([](const Idevice* l, const Idevice *r) -> bool
	{
		return l->start_addr() <= r->start_addr();
	});
}

bool DeviceHub::device_addr_range_overlap(const Idevice *l, const Idevice *r)
{
	if (l->start_addr() >= r->end_addr())
		return false;
	if (r->start_addr() >= l->end_addr())
		return false;
	return true;
}

Idevice* DeviceHub::find_device_by_addr(gpa addr)
{
	for (auto i : m_devices) {
		if (addr < i->start_addr())
			continue;
		if (addr >= i->end_addr())
			continue;
		return i;
	}

	return NULL;
}

bool DeviceHub::incorrect_size(u64 size)
{
	if (size > 8)
		return true;
	return false;
}
