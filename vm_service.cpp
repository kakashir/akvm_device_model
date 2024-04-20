#include <errno.h>
#include <cstdio>
#include "common.hpp"
#include "vm_service.hpp"
#include "device_hub.hpp"

#define VM_SERVICE_DEBUG 0xfeULL
#define VM_SERVICE_PANIC 0xffULL

#define VM_SERVICE_IO 0x1
#define VM_SERVICE_IO_PORT_IN  0x1
#define VM_SERVICE_IO_PORT_OUT 0x2
#define VM_SERVICE_IO_MMIO_IN 0x3
#define VM_SERVICE_IO_MMIO_OUT 0x4

static int handle_vm_service_debug_panic(u64 type,
					 u64 *in_out, u64 in_out_count)
{
	printf("vcpu %s:\n",
	       type == VM_SERVICE_DEBUG ? "DEBUG" : "PANIC");
	printf("  arg0: 0x%lx\n", in_out[0]);
	printf("  arg1: 0x%lx\n", in_out[1]);
	printf("  arg2: 0x%lx\n", in_out[2]);
	printf("  arg3: 0x%lx\n", in_out[3]);
	printf("  arg4: 0x%lx\n", in_out[4]);
	printf("  arg5: 0x%lx\n", in_out[5]);

	return type == VM_SERVICE_DEBUG ? 0 : 1;
}

static int handle_vm_service_io(u64 *in_out, u64 in_out_count)
{
	DeviceHub &device_hub = DeviceHub::instance();
	u64 type = in_out[0];
	gpa addr = in_out[1];
	u64 size = in_out[2];

	switch(type) {
	case VM_SERVICE_IO_PORT_IN:
		return device_hub.handle_port_read(addr, size, in_out[3]);
	case VM_SERVICE_IO_PORT_OUT:
		return device_hub.handle_port_write(addr, size, in_out[3]);
	case VM_SERVICE_IO_MMIO_IN:
		return device_hub.handle_mmio_read(addr, size, in_out[3]);
	case VM_SERVICE_IO_MMIO_OUT:
		return device_hub.handle_mmio_write(addr, size, in_out[3]);
	}

	printf("IO failed: addr:0x%lx type:0x%x size:0x%llx val:0x%llx\n",
	       addr, type, size, in_out[3]);
	return -ENOTSUP;
}

int handle_vm_service(u64 type, u64 *in_out, u64 in_out_count)
{
	switch(type) {
	case VM_SERVICE_IO:
		return handle_vm_service_io(in_out, in_out_count);
	case VM_SERVICE_DEBUG:
	case VM_SERVICE_PANIC:
		return handle_vm_service_debug_panic(type, in_out, in_out_count);
	default:
		printf("unsupported vm service type: %d\n", type);
		return -ENOTSUP;
	}
}
