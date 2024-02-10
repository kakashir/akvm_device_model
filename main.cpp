#include <iostream>
#include <cstdlib>

#include "common.hpp"
#include "cpu.hpp"
#include "device.hpp"
#include "memory.hpp"
#include "accel.hpp"
#include "platform.h"

Akvm g_akvm;
Memory *memory;

static struct memory_config {
	gpa gpa_start;
	size_t size;
} g_mem_config[] = {
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

static int g_mem_config_count = sizeof(g_mem_config)/sizeof(g_mem_config[0]);

static int __alloc_memory(struct Memory *memory,
			  struct memory_config *mem_config, int count)
{
	int r;
	for (int i = 0; i < count; ++i) {
		r = memory[i].alloc_memory(mem_config[i].gpa_start,
					   mem_config[i].size,
					   AKVM_MEMORY_SLOT_ALIGN);
		if (r)
			return r;
	}

	return 0;
}

static int __install_memory(Akvm &akvm, Memory *memory, int count)
{
	int r;
	for (int i = 0; i < count; ++i) {
		r = akvm.add_memory(memory[i].gpa_start(), memory[i].size(),
				    memory[i].hva_start());
		if (r)
			return r;
	}
	return 0;
}

static int __load_guest_code(const char *path, void *p, long size)
{
	FILE *fp;
	long guest_size;
	size_t io_size;
	int r = 0;

	fp = fopen(path, "r");
	if (!fp) {
		printf("failed to open guest file: %d\n", errno);
		return -1;
	}

	fseek(fp, 0, SEEK_END);
	guest_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	if (guest_size > size) {
		printf("guest file too big: %d > %d\n", guest_size, size);
		r = -2;
		goto exit_fp;
	}
	io_size = fread(p, guest_size, 1, fp);
	if (io_size != 1) {
		printf("failed to read guest code, io size: %ld\n", io_size);
		r = -3;
		goto exit_fp;
	}
 exit_fp:
	fclose(fp);
	return r;
}

int main(int argc, char* argv[])
{
	int r;
	Cpu cpu;

	r = g_akvm.initialize();
	if (r) {
		printf("g_akvm: initialize failed: %d\n", r);
		return r;
	}

	memory = new Memory[g_mem_config_count];
	if (!memory) {
		printf("g_akvm: initialize memory array failed\n");
		return -ENOMEM;
	}

	r = __alloc_memory(memory, g_mem_config, g_mem_config_count);
	if (r) {
		printf("memory allocat failed: %d\n", r);
		return r;
	}

	r = __load_guest_code("/home/yy/src/ld_script/binary",
			      reinterpret_cast<void*>(memory[0].hva_start()),
			      memory[0].size());
	if (r) {
		printf("load guest code failed: %d\n", r);
		return r;
	}

	r = g_akvm.create_vm();
	if (r) {
		printf("akvm create vm failed: %d\n", r);
		return r;
	}

	r = __install_memory(g_akvm, memory, g_mem_config_count);
	if (r) {
		printf("akvm add memory failed: %d\n", r);
		return r;
	}

	r = cpu.create(&g_akvm);
	if (r) {
		printf("cpu create failed: %d\n", r);
		return r;
	}

	cpu.run();
	cpu.wait();

	delete[] memory;

	return 0;
}
