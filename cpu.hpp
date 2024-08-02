#ifndef __CPU_HPP
#define __CPU_HPP

#include <pthread.h>
#include <semaphore.h>

#include "common.hpp"
#include "accel.hpp"

interface Icpu {
	virtual ~Icpu() {;}
	virtual int create(Akvm *accel) = 0;
	virtual int setup(void) = 0;
	virtual int run(void) = 0;
	virtual void destroy(void) = 0;
	virtual void wait(void) = 0;
	virtual void set_startup_rip(gpa val) = 0;
};

class Cpu : public Icpu {
public:
	Cpu(void);
	~Cpu(void);
public:
	virtual int create(Akvm *accel);
	virtual int setup(void);
	virtual int run(void);
	virtual void destroy(void);
	virtual void wait(void);
	virtual void set_startup_rip(gpa val)
	{ m_startup_rip = val; }
private:
	int handle_exit(struct akvm_vcpu_runtime *runtime);
	static void* vcpu_thread(void *cpu);
	int setup_cpuid(void);
	bool find_supported_cpuid(int leaf, int sub_leaf,
				  akvm_cpuid_entry &data);
private:
	pthread_t m_thread;
	sem_t m_run;
	sem_t m_stop;
	Akvm *m_accel;
	volatile bool m_should_exit;
	bool m_created;
	bool m_running;
	gpa m_startup_rip;
	struct akvm_vcpu_runtime *m_vcpu_runtime;

};

#endif
