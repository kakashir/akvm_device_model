#ifndef _DEVICE_HWCFG_HPP
#define _DEVICE_HWCFG_HPP

#include <vector>
#include "common.hpp"
#include "platform.h"
#include "device.hpp"

class DeviceHwcfg : public Idevice
{
public:
	DeviceHwcfg(void);
	virtual ~DeviceHwcfg(void);

public:
	int handle_port_write(gpa addr, u64 size, u64 val);
	int handle_port_read(gpa addr, u64 size, u64 &val);
	int handle_mmio_write(gpa addr, u64 size, u64 val);
	int handle_mmio_read(gpa addr, u64 size, u64 &val);

	gpa start_addr(void) const
	{ return DEVICE_HWCFG_MMIO_BEGIN; }

	gpa end_addr(void) const
	{ return DEVICE_HWCFG_MMIO_END; }

private:
	int select_type(u64 val);
	int do_ctl(u64 val);

	int select_type_memory(void);
	int do_ctl_memory(u64 val);
	int type_memory_read(gpa addr, u64 size, u64 &val);
private:
	struct {
		std::vector<hwcfg_memory_info> data;
		std::vector<hwcfg_memory_info>::iterator iter;
	} m_memory;
	u64 m_type;
};


#endif
