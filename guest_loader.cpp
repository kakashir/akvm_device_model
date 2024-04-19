#include <iostream>
#include <cstring>
#include <cstdlib>
#include <elf.h>
#include "guest_loader.hpp"
#include "memory.hpp"

int load_elf_guest_code(const char *path, MemoryHub &memoryhub,
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
	Imemory *mem;
	std::vector<Imemory*> found;

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

		if (!memoryhub.find_memory(elf_phdr.p_paddr,
					   std::max(elf_phdr.p_memsz, elf_phdr.p_filesz), found)
		    || found.size() > 1) {
			r = -8;
			goto exit_fp;
		}

		mem = found[0];
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
