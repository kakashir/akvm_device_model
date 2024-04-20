#include <iostream>
#include "common.hpp"
#include "platform.h"
#include "cpu.hpp"
#include "device.hpp"
#include "memory.hpp"
#include "accel.hpp"
#include "memoryhub.hpp"
#include "guest_loader.hpp"

Akvm g_akvm;

std::vector<memory_config> g_mem_config = {
	{ .gpa_start = MEM_BELOW_1M_START,
	  .size = MEM_BELOW_1M_SIZE
	},
	{ .gpa_start = MEM_BELOW_4G_START,
	  .size = MEM_BELOW_4G_END - MEM_BELOW_4G_START
	},
	{ .gpa_start = MEM_ABOVE_4G_START,
	  .size = 1ULL * 1024  * 1024 * 1024
	},
};

static int install_memory(MemoryHub &memoryhub, Akvm &akvm)
{
	struct T {
		T(Akvm &akvm):_akvm(akvm),r(0) {;}
		void operator() (Imemory *memory) {
			int _r = _akvm.add_memory(memory->gpa_start(),
						  memory->size(),
						  memory->hva_start());
			r |= _r;
		}
		Akvm &_akvm;
		int r;
	} installer(akvm);

	memoryhub.for_each(installer);
	return installer.r;
}

static void __dump_akvm_supported_cpuid(void)
{
	struct akvm_cpuid *cpuid;

	cpuid = g_akvm.get_supported_cpuid();
	if (!cpuid) {
		printf("failed to got cpuid\n");
		return;
	}

	printf("Dump AKVM supported cpuid:\n");
	for (int i = 0; i < cpuid->count; ++i)
		printf("l:0%x s:0x%x eax:0x%x ebx:0x%x ecx:0x%x edx:0x%x\n",
		       cpuid->entry[i].leaf,
		       cpuid->entry[i].sub_leaf,
		       cpuid->entry[i].eax, cpuid->entry[i].ebx,
		       cpuid->entry[i].ecx, cpuid->entry[i].edx);
	printf("Dump AKVM supported cpuid end\n");
}

int main(int argc, char* argv[])
{
	int r;
	Cpu cpu;
	gpa startup_rip;

	r = g_akvm.initialize();
	if (r) {
		printf("g_akvm: initialize failed: %d\n", r);
		return r;
	}
	__dump_akvm_supported_cpuid();

	r = get_memoryhub().alloc_memory(g_mem_config);
	if (r) {
		printf("memory allocat failed: %d\n", r);
		return r;
	}

	r = load_elf_guest_code("/home/yy/src/akvm_guest/binary",
				get_memoryhub(), startup_rip);
	if (r) {
		printf("load guest code failed: %d\n", r);
		return r;
	}

	r = g_akvm.create_vm();
	if (r) {
		printf("akvm create vm failed: %d\n", r);
		return r;
	}

	r = install_memory(get_memoryhub(), g_akvm);
	if (r) {
		printf("akvm add memory failed: %d\n", r);
		return r;
	}

	r = cpu.create(&g_akvm);
	if (r) {
		printf("cpu create failed: %d\n", r);
		return r;
	}

	cpu.set_startup_rip(startup_rip);
	cpu.run();
	cpu.wait();

	return 0;
}
