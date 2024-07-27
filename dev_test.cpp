#include <iostream>
#include "common.hpp"
#include "platform.h"
#include "cpu.hpp"
#include "device.hpp"
#include "memory.hpp"
#include "accel.hpp"
#include "device_serial.hpp"
#include "device_hwcfg.hpp"
#include "memoryhub.hpp"
#include "device_hub.hpp"

#include <unistd.h>
#include <cstdlib>
#include <sys/ioctl.h>
#include <string.h>

static void* memory_slot_replace_test_thread(void *unused)
{
	unsigned long i;
	int r;
	struct akvm_memory_slot slot;
	struct akvm_memory_slot old;
	Akvm *akvm = reinterpret_cast<Akvm*>(unused);
	unsigned long val;
	void *p;

	printf("****** Memory slot replacement test begin ******\n");
	for(i = 1; i; ++i) {
		p = std::aligned_alloc(AKVM_MEMORY_SLOT_ALIGN,
				       256ULL * 1024 * 1024);
		if (i & 1)
			val = 0x1234567800000000ULL;
		else
			val = 0x1234567887654321ULL;
		*((unsigned long*)p) = val;

		slot.hva = (__u64)p;
		slot.gpa = MEM_ABOVE_4G_START;
		slot.size = 256ULL * 1024 * 1024;
		slot.flags = 0;

		do {
			r = akvm->replace_memory(slot, old);
			if (r)
				usleep(100000);
		} while(r);

		if (old.gpa != MEM_ABOVE_4G_START || old.size != 256ULL * 1024 * 1024) {
			printf("INCORRECT old slot: hva:0x%llx gpa:0x%llx size:0x%llx\n", old.hva, old.gpa, old.size);
			exit(-1);
		}
		std::free((void*)old.hva);
	}
	printf("****** Memory slot replacement test end ******\n");
	return NULL;
}

void dev_test_memory_slot_repalce(Akvm &akvm)
{
	static pthread_t t_thread;

	pthread_create(&t_thread, NULL,
		       memory_slot_replace_test_thread, (void*)&akvm);
}
