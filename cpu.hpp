#ifndef __CPU_HPP
#define __CPU_HPP

#include <pthread.h>
#include <semaphore.h>

#include "common.hpp"

interface Icpu {
	virtual ~Icpu() {;}
	virtual int create(void *argc) = 0;
	virtual int run(void) = 0;
	virtual void destroy(void) = 0;
	virtual void wait(void) = 0;
};

class Cpu : public Icpu {
public:
	Cpu(void);
	~Cpu(void);
public:
	virtual int create(void *argc);
	virtual int run(void);
	virtual void destroy(void);
	virtual void wait(void);
private:
	static void* vcpu_thread(void *cpu);
private:
	pthread_t m_thread;
	sem_t m_run;
	void *m_argc;
	volatile bool m_should_exit;
	bool m_created;
	bool m_running;

};

#endif
