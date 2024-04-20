#include <errno.h>
#include <cstdio>
#include "common.hpp"
#include "vm_service.hpp"
#include "device_hub.hpp"

#define VM_SERVICE_TYPE_DEBUG 0xfeULL
#define	VM_SERVICE_TYPE_PANIC 0xffULL

static int handle_vm_service_debug_panic(u64 type,
					 u64 *in_out, u64 in_out_count)
{
	printf("vcpu %s:\n",
	       type == VM_SERVICE_TYPE_DEBUG ? "DEBUG" : "PANIC");
	printf("  arg0: 0x%lx\n", in_out[0]);
	printf("  arg1: 0x%lx\n", in_out[1]);
	printf("  arg2: 0x%lx\n", in_out[2]);
	printf("  arg3: 0x%lx\n", in_out[3]);
	printf("  arg4: 0x%lx\n", in_out[4]);
	printf("  arg5: 0x%lx\n", in_out[5]);

	return type == VM_SERVICE_TYPE_DEBUG ? 0 : 1;
}

int handle_vm_service(u64 type, u64 *in_out, u64 in_out_count)
{
	switch(type) {
	case VM_SERVICE_TYPE_DEBUG:
	case VM_SERVICE_TYPE_PANIC:
		return handle_vm_service_debug_panic(type, in_out, in_out_count);
	default:
		printf("unsupported vm service type: %d\n", type);
		return -ENOTSUP;
	}
}
