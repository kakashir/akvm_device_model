#include <iostream>
#include <cstdlib>
#include <elf.h>
#include <string.h>

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

static int __alloc_memory(Memory *memory,
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

static Memory* __find_memory_range(Memory *memory, int count,
					  gpa start, gpa size)
{
	for (int i = 0; i < count; ++i) {
		gpa end = start + size;
		gpa memory_start = memory[i].gpa_start();
		gpa memory_end = memory[i].gpa_start() + memory[i].size();

		if (start >= memory_start && end < memory_end)
			return &memory[i];
	}
	return NULL;
}

static int __load_elf_guest_code(const char *path, struct Memory *memory, int count,
				 gpa &startup_rip)
{
	int r = 0;
	FILE *fp;
	long guest_size;
	size_t io_size;
	Elf64_Ehdr elf_hdr;
	Elf64_Phdr elf_phdr;
	unsigned char *guest_mem;
	gpa guest_mem_start;
	Memory *mem;

	fp = fopen(path, "r");
	if (!fp)
		return -1;

	fseek(fp, 0, SEEK_END);
	guest_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	if (guest_size < sizeof(elf_hdr)) {
		r = -2;
		goto exit_fp;
	}

	io_size = fread(&elf_hdr, sizeof(elf_hdr), 1, fp);
	if (io_size != 1) {
		r = -3;
		goto exit_fp;
	}

	if (elf_hdr.e_ident[0] != ELFMAG0 || elf_hdr.e_ident[1] != ELFMAG1 ||
	    elf_hdr.e_ident[2] != ELFMAG2 || elf_hdr.e_ident[3] != ELFMAG3) {
		r = -4;
		goto exit_fp;
	}

	if (elf_hdr.e_ident[4] != ELFCLASS64) {
		r = -5;
		goto exit_fp;
	}

	for (int i = 0; i < elf_hdr.e_phnum; ++i) {
		r = fseek(fp, elf_hdr.e_phoff + (i * elf_hdr.e_phentsize),
			  SEEK_SET);
		if (r) {
			r = -6;
			goto exit_fp;
		}

		io_size = fread(&elf_phdr, elf_hdr.e_phentsize, 1, fp);
		if (io_size != 1) {
			r = -7;
			goto exit_fp;
		}

		if (elf_phdr.p_type != PT_LOAD)
			continue;

		mem = __find_memory_range(memory, count, elf_phdr.p_paddr,
					  std::max(elf_phdr.p_memsz, elf_phdr.p_filesz));
		if (!mem) {
			r = -8;
			goto exit_fp;
		}
		guest_mem = reinterpret_cast<unsigned char*>(mem->hva_start());
		guest_mem_start = mem->gpa_start();

		/* load normal section like .data/.text */
		if (elf_phdr.p_filesz) {
			r = fseek(fp, elf_phdr.p_offset, SEEK_SET);
			if (r) {
				r = -9;
				goto exit_fp;
			}
			io_size = fread(guest_mem + elf_phdr.p_paddr - guest_mem_start,
					elf_phdr.p_filesz, 1, fp);
			if (io_size != 1) {
				r = -10;
				goto exit_fp;
			}
		}

		/* handle .bss section */
		if (elf_phdr.p_memsz > elf_phdr.p_filesz)
			memset(guest_mem + elf_phdr.p_paddr + elf_phdr.p_filesz - guest_mem_start,
			       0, elf_phdr.p_memsz - elf_phdr.p_filesz);

		printf("Loaded %d: type:0x%x offset:0x%lx paddr:0x%lx vaddr:0x%lx file_size:0x%lx mem_size:0x%lx align:0x%lx to [0x%lx, 0x%lx)\n",
		       i, elf_phdr.p_type, elf_phdr.p_offset,
		       elf_phdr.p_paddr, elf_phdr.p_vaddr,
		       elf_phdr.p_filesz, elf_phdr.p_memsz,
		       elf_phdr.p_align, mem->gpa_start(), mem->gpa_start() + mem->size());
	}

exit_fp:
	startup_rip = elf_hdr.e_entry;
	fclose(fp);
	return r;
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

	r = __load_elf_guest_code("/home/yy/src/akvm_guest/binary",
				  memory, g_mem_config_count, startup_rip);
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

	cpu.set_startup_rip(startup_rip);
	cpu.run();
	cpu.wait();

	delete[] memory;

	return 0;
}
