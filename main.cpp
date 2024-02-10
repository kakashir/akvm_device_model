#include <iostream>
#include <cstdlib>

#include "common.hpp"
#include "cpu.hpp"
#include "device.hpp"
#include "memory.hpp"
#include "accel.hpp"

Akvm g_akvm;
Memory g_memory;

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

	r = g_memory.alloc_memory(0, 1024 * 1024, AKVM_MEMORY_SLOT_ALIGN);
	if (r) {
		printf("memory allocat failed: %d\n", r);
		return r;
	}

	r = __load_guest_code("/home/yy/src/ld_script/binary",
			      reinterpret_cast<void*>(g_memory.hva_start()),
			      g_memory.size());
	if (r) {
		printf("load guest code failed: %d\n", r);
		return r;
	}

	r = g_akvm.create_vm();
	if (r) {
		printf("akvm create vm failed: %d\n", r);
		return r;
	}
	r = g_akvm.add_memory(g_memory.gpa_start(), g_memory.size(),
			      g_memory.hva_start());
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

	return 0;
}
