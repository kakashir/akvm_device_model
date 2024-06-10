#include <sys/ioctl.h>
#include <sys/mman.h>

#include <unistd.h>
#include <fcntl.h>
#include <cerrno>
#include <cstdlib>
#include <cstdio>

#include "accel.hpp"
#include "linux_header/include/linux/akvm.h"

#define AKVM_PATH "/dev/akvm"

Akvm::Akvm():vcpu_fd(-1)
{;}

Akvm::~Akvm()
{;}

int Akvm::initialize(void)
{
	int fd;

	if (Akvm::dev_fd >= 0)
		return 0;

	fd = open(AKVM_PATH, 0);
	if (fd < 0)
		return fd;

	Akvm::dev_fd = fd;
	return 0;
}

void Akvm::destroy(void)
{
	if (Akvm::dev_fd >= 0) {
		close(Akvm::dev_fd);
		Akvm::dev_fd = -1;
	}

	if (Akvm::vm_fd >= 0) {
		close(Akvm::vm_fd);
		Akvm::vm_fd = -1;
	}

	if (vcpu_fd >= 0) {
		close(vcpu_fd);
		vcpu_fd = -1;
	}
}

struct akvm_cpuid* Akvm::get_supported_cpuid(void)
{
	struct akvm_cpuid *data = NULL;
	struct akvm_cpuid *null = NULL;
	struct akvm_cpuid *r_cpuid;
	int r;

	if (cpuid)
		return cpuid;

	data = (akvm_cpuid*)calloc(1, sizeof(*data));
	if (!data) {
		printf("Failed to memory alloctation cpuid data\n");
		_exit(-1);
	}

	data->count = 1;
	do  {
		data->count *= 2;
		free(data->entry);
		data->entry = (struct akvm_cpuid_entry *)
			malloc(data->count * sizeof(data->entry[0]));
		if (!data->entry) {
			printf("Failed to memory alloctation cpuid entry\n");
			_exit(-1);
		}
		printf("data->count:%d\n", data->count);

		r = ioctl(Akvm::dev_fd, AKVM_GET_CPUID, data);
	} while (r < 0 && errno == E2BIG);

	if (r < 0) {
		printf("Error code: %d\n", errno);
		r_cpuid = NULL;
		goto exit_free;
	}

	if (__atomic_compare_exchange_n(&cpuid, &null, data, false,
					__ATOMIC_RELAXED, __ATOMIC_RELAXED))
		return data;

	r_cpuid = cpuid;
exit_free:
	free(data->entry);
	free(data);
	return r_cpuid;
}

int Akvm::set_vcpu_cpuid(struct akvm_cpuid *cpuid)
{
	if (!cpuid)
		return -EINVAL;
	if (vcpu_fd < 0)
		return -EINVAL;

	return ioctl(vcpu_fd, AKVM_VCPU_SET_CPUID, cpuid);
}

int Akvm::create_vm(void)
{
	int fd;

	if (Akvm::dev_fd < 0)
		return -EINVAL;

	fd = ioctl(Akvm::dev_fd, AKVM_CREATE_VM);
	if (fd < 0)
		return fd;

	Akvm::vm_fd = fd;
	return 0;
}

int Akvm::create_vcpu(void)
{
	int fd;

	if (Akvm::vm_fd < 0)
		return -EINVAL;

	fd = ioctl(Akvm::vm_fd, AKVM_CREATE_VCPU);
	if (fd < 0)
		return fd;

	vcpu_fd = fd;
	return 0;

}

int Akvm::run_vcpu(void)
{
	if (vcpu_fd < 0)
		return -EINVAL;
	return ioctl(vcpu_fd, AKVM_RUN);
}

int Akvm::add_memory(gpa gpa_start, size_t size, hva hva_start)
{
	struct akvm_memory_slot slot = {
		.hva = hva_start,
		.gpa = gpa_start,
		.size = size,
		.flags = 0,
	};

	if (Akvm::vm_fd < 0)
		return -EINVAL;

	return ioctl(Akvm::vm_fd, AKVM_MEMORY_SLOT_ADD, &slot);
}

int Akvm::remove_memory(gpa gpa_start, size_t size, hva hva_start)
{
	struct akvm_memory_slot slot = {
		.hva = hva_start,
		.gpa = gpa_start,
		.size = size,
		.flags = 0,
	};

	if (Akvm::vm_fd < 0)
		return -EINVAL;

	return ioctl(Akvm::vm_fd, AKVM_MEMORY_SLOT_REMOVE, &slot);
}

int Akvm::get_vcpu_runtime_info(struct akvm_vcpu_runtime** runtime)
{
	struct akvm_vcpu_runtime* rt;

	if (vcpu_fd < 0)
		return -EINVAL;
	if (!runtime)
		return -EINVAL;

	rt = reinterpret_cast<struct akvm_vcpu_runtime*>(
		mmap(NULL, sizeof(*runtime), PROT_READ | PROT_WRITE,
		     MAP_SHARED, vcpu_fd,
		     AKVM_VCPU_RUNTIME_PG_OFF));

	if (rt == MAP_FAILED)
		return -errno;

	*runtime = rt;
	return 0;
}

void Akvm::set_startup_rip(gpa rip)
{
	if (vcpu_fd < 0)
		return;
	ioctl(vcpu_fd, AKVM_VCPU_SET_RIP, rip);
	return;
}

int Akvm::dev_fd = -1;
int Akvm::vm_fd = -1;
struct akvm_cpuid* Akvm::cpuid;
