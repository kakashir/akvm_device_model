#include "utility.hpp"

void raw_cpuid(int leaf, int sub_leaf,
	       unsigned int &eax, unsigned int &ebx,
	       unsigned int &ecx, unsigned int &edx)
{
	asm volatile("cpuid"
		     :"=a"(eax),"=b"(ebx),"=c"(ecx),"=d"(edx)
		     :"a"(leaf),"c"(sub_leaf));
}
