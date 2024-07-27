#ifndef __ACCEL_H
#define __ACCEL_H

#include "common.hpp"
#include "linux_header/include/linux/akvm.h"

class Akvm
{
public:
	Akvm();
	~Akvm();
public:
	int initialize(void);
	void destroy(void);
	struct akvm_cpuid* get_supported_cpuid(void);
	int set_vcpu_cpuid(struct akvm_cpuid *cpuid);
	int create_vm(void);
	int create_vcpu(void);
	int run_vcpu(void);
	int add_memory(gpa gpa_start, size_t size, hva hva_start);
	int remove_memory(gpa gpa_start, size_t size, hva hva_start);
	int replace_memory(struct akvm_memory_slot replace,
			   struct akvm_memory_slot &old);
	int get_vcpu_runtime_info(struct akvm_vcpu_runtime** runtime);
	void set_startup_rip(gpa rip);
private:
	static int dev_fd;
	static int vm_fd;
	static struct akvm_cpuid *cpuid;
	int vcpu_fd;
};

#endif
