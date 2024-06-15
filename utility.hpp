#ifndef __UTILITY_H
#define __UTILITY_H

#include "common.hpp"

void raw_cpuid(int leaf, int sub_leaf,
	       unsigned int &eax, unsigned int &ebx,
	       unsigned int &ecx, unsigned int &edx);

#endif
