#include <cerrno>
#include <iostream>
#include <vector>
#include "common.hpp"
#include "cpu.hpp"
#include "accel.hpp"
#include "vm_service.hpp"
#include "utility.hpp"

Cpu::Cpu(void):m_should_exit(false),
	       m_created(false),m_running(false),
	       m_accel(NULL),
	       m_vcpu_runtime(NULL),
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

	r = accel->create_vcpu();
	if (r)
		return r;

	r = accel->get_vcpu_runtime_info(&m_vcpu_runtime);
	if (r)
		return r;

	m_created = true;
	m_running = false;
	m_accel = accel;
	return 0;
}

int Cpu::setup(void)
{
	return setup_cpuid();
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
	Cpu *cpu;
	Akvm *accel;
	struct akvm_vcpu_runtime *rt;

	cpu = reinterpret_cast<Cpu*>(_cpu);

	sem_wait(&cpu->m_run);

	/*
	  Only get data after ordered to run.
	  Other wise 'use before initialization'.
	 */

	rt = cpu->m_vcpu_runtime;
	accel = cpu->m_accel;

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

int Cpu::setup_cpuid(void)
{
	std::vector<struct akvm_cpuid_entry> tmp_cpuid;
	struct akvm_cpuid akvm_cpuid;
	struct akvm_cpuid_entry e;
	struct akvm_cpuid_entry *p;
	unsigned int max_leaf;
	unsigned int extend_pass_thru_cpuid[] = {
		0x80000000, 0x80000001, 0x80000007, 0x80000008
	};
	unsigned int extend_raw_cpuid[] = {
		0x80000002, 0x80000003, 0x80000004, 0x80000005,	0x80000006,
	};
	int r;

	if (!find_supported_cpuid(0x80000000, 0, e)) {
		printf("skip extend cpuid setup: 0x80000000 not supported\n");
		return 0;
	}

	max_leaf = e.eax;
	for (unsigned int leaf : extend_pass_thru_cpuid) {
		if (leaf > max_leaf) {
			printf("Skip setup cpuid: 0x%x, exceed max supported leaf number 0x%x\n",
			       leaf, max_leaf);
			continue;
		}

		if (!find_supported_cpuid(leaf, 0, e)) {
			printf("Skip setup cpuid: 0x%x, not supported by accelerator\n",
			       leaf);
			continue;
		}
		tmp_cpuid.push_back(e);
	}

	for (unsigned int leaf : extend_raw_cpuid) {
		if (leaf > max_leaf) {
			printf("Skip setup cpuid: 0x%x, exceed max supported leaf number 0x%x\n",
			       leaf, max_leaf);
			continue;
		}

		raw_cpuid(leaf, 0, e.eax, e.ebx, e.ecx, e.edx);
		e.leaf = leaf;
		e.sub_leaf = 0;
		tmp_cpuid.push_back(e);
	}

	if (tmp_cpuid.empty())
		return -1;

	akvm_cpuid.entry = p = (struct akvm_cpuid_entry*)
		malloc(sizeof(*akvm_cpuid.entry) * tmp_cpuid.size());
	for (const struct akvm_cpuid_entry &i : tmp_cpuid)
		*p++ = i;
	akvm_cpuid.count = tmp_cpuid.size();

	r = m_accel->set_vcpu_cpuid(&akvm_cpuid);

	free(akvm_cpuid.entry);
	return r;
}

bool Cpu::find_supported_cpuid(int leaf, int sub_leaf,
			       akvm_cpuid_entry &data)
{
	struct akvm_cpuid *cpuid;

	if (!m_accel)
		return false;

	cpuid = m_accel->get_supported_cpuid();
	if (!cpuid)
		return false;

	for (int i = 0; i < cpuid->count; ++i) {
		if (cpuid->entry[i].leaf != leaf)
			continue;
		if (cpuid->entry[i].sub_leaf != sub_leaf)
			continue;
		data = cpuid->entry[i];
		return true;
	}

	return false;
}
