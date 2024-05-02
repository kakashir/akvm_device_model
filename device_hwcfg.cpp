#include <errno.h>
#include <cstring>
#include <cstdio>
#include "device_hwcfg.hpp"
#include "memory.hpp"
#include "memoryhub.hpp"

DeviceHwcfg::DeviceHwcfg(void)
{}

DeviceHwcfg::~DeviceHwcfg(void)
{
	m_memory.data.clear();
}

int DeviceHwcfg::handle_port_write(gpa addr, u64 size, u64 val)
{
	return -EIO;
}

int DeviceHwcfg::handle_port_read(gpa addr, u64 size, u64 &val)
{
	return -EIO;
}

int DeviceHwcfg::select_type_memory(void)
{
	MemoryHub &h = MemoryHub::instance();

	m_type = HWCFG_MEMORY_INFO;
	h.for_each([this](Imemory *e) {
		this->m_memory.data.push_back(
			{
				.addr = e->gpa_start(),
				.size = e->size(),
				.type = HWCFG_PHYSICAL_MEMORY,
			});
	});

	m_memory.iter = m_memory.data.begin();

	if (!this->m_memory.data.empty())
		return 0;
	return -EIO;
}

int DeviceHwcfg::select_type(u64 val)
{
	switch (val) {
	case HWCFG_MEMORY_INFO:
		return select_type_memory();
	default:
		return -EIO;
	}
}

int DeviceHwcfg::do_ctl_memory(u64 val)
{
	if (val & HWCFG_MEMORY_CTL_NEXT) {
		if (m_memory.iter != m_memory.data.end())
			++m_memory.iter;
		return 0;
	}
	return -EIO;
}

int DeviceHwcfg::do_ctl(u64 val)
{
	switch (m_type) {
	case HWCFG_MEMORY_INFO:
		return do_ctl_memory(val);
	default:
		return -EIO;
	}
}

int DeviceHwcfg::handle_mmio_write(gpa addr, u64 size, u64 val)
{
	switch (addr) {
	case DEVICE_HWCFG_REG_TYPE:
		return select_type(val);
	case DEVICE_HWCFG_REG_CTL:
		return do_ctl(val);
	default:
		return -EIO;
	}
}

int DeviceHwcfg::type_memory_read(gpa addr, u64 size, u64 &val)
{
	hwcfg_memory_info r;

	if (size != 8)
		return -EIO;

	if (addr < DEVICE_HWCFG_REG_DATA_0 || addr > DEVICE_HWCFG_REG_DATA_3)
		return -EIO;

	if (m_memory.iter > m_memory.data.end())
		memset(&r, 0, sizeof(r));
	else
		r = *m_memory.iter;

	switch (addr) {
	case DEVICE_HWCFG_REG_DATA_0:
		val = r.addr;
		break;
	case DEVICE_HWCFG_REG_DATA_1:
		val = r.size;
		break;
	case DEVICE_HWCFG_REG_DATA_2:
		val = r.type;
		break;
	case DEVICE_HWCFG_REG_DATA_3:
		val = r.flag;
		break;
	}

	return 0;
}

int DeviceHwcfg::handle_mmio_read(gpa addr, u64 size, u64 &val)
{
	switch (m_type) {
	case HWCFG_MEMORY_INFO:
		return type_memory_read(addr, size, val);
	default:
		return -EIO;
	}
}
