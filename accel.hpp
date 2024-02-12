#ifndef __ACCEL_H
#define __ACCEL_H

#include "common.hpp"

class Akvm
{
public:
	Akvm();
	~Akvm();
public:
	int initialize(void);
	void destroy(void);

	int create_vm(void);
	int create_vcpu(void);
	int run_vcpu(void);
	int add_memory(gpa gpa_start, size_t size, hva hva_start);
	int remove_memory(gpa gpa_start, size_t size, hva hva_start);
	int get_vcpu_runtime_info(struct akvm_vcpu_runtime** runtime);
	void set_startup_rip(gpa rip);
private:
	static int dev_fd;
	static int vm_fd;
	int vcpu_fd;
};

#endif
