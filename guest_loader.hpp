#ifndef __GUEST_LOADER_HPP
#define __GUEST_LOADER_HPP

#include "common.hpp"
#include "memoryhub.hpp"

int load_elf_guest_code(const char *path, MemoryHub &memoryhub,
			gpa &startup_rip);

#endif
