#include <cerrno>
#include <iostream>
#include "common.hpp"
#include "cpu.hpp"
#include "accel.hpp"
#include "vm_service.hpp"

Cpu::Cpu(void):m_should_exit(false),
	       m_created(false),m_running(false),
	       m_accel(NULL),
	       m_startup_rip(0)
{;}

Cpu::~Cpu(void)
{
	destroy();
}

int Cpu::create(Akvm *accel)
{
	int r;

	sem_init(&m_run, 0, 0);
	r = pthread_create(&m_thread, NULL,
			   vcpu_thread, this);
	if (r)
		return r;

	m_created = true;
	m_running = false;
	m_accel = accel;
	return 0;
}

int Cpu::run(void)
{
	if (!m_created)
		return -EINVAL;
	if (m_running)
		return -EINVAL;

	sem_post(&m_run);
	m_running = true;
	return 0;
}

void Cpu::destroy(void)
{
	if (!m_created)
		return;
	m_should_exit = true;
	if (!m_running)
		sem_post(&m_run);
	else
		sem_wait(&m_run);
	m_created = false;
	m_running = false;
}

void Cpu::wait(void)
{
	void *unused;
	if (m_created)
		pthread_join(m_thread, &unused);
}

int Cpu::handle_exit(struct akvm_vcpu_runtime *runtime)
{
	switch(runtime->exit_reason) {
	case AKVM_EXIT_VM_SERVICE:
		runtime->vm_service.ret =
			handle_vm_service(runtime->vm_service.type,
					  runtime->vm_service.in_out,
					  runtime->vm_service.in_out_count);
		return 0;
	default:
		printf("Unimplemented exit reason: %d\n", runtime->exit_reason);
		return -ENOTSUP;
	}
}

void* Cpu::vcpu_thread(void *_cpu)
{
	int r;
	struct akvm_vcpu_runtime *rt;
	Cpu *cpu = reinterpret_cast<Cpu*>(_cpu);
	Akvm *accel = cpu->m_accel;

	sem_wait(&cpu->m_run);

	r = accel->create_vcpu();
	if (r)
		goto exit;

	r = accel->get_vcpu_runtime_info(&rt);
	if (r)
		goto exit;

	accel->set_startup_rip(cpu->m_startup_rip);
	while(!cpu->m_should_exit) {
		if (r) {
			cpu->m_should_exit = true;
			continue;
		}

		r = accel->run_vcpu();
		if (!r)
			continue;
		if (r < 0) {
			printf("run_vcpu: r:%d errno:%d\n", r, -errno);
			cpu->m_should_exit = true;
			continue;
		}
		r = cpu->handle_exit(rt);
	}
exit:
	sem_post(&cpu->m_run);
	return NULL;
}
