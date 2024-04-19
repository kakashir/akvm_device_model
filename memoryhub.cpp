#include <errno.h>
#include "memoryhub.hpp"
#include "memory.hpp"

MemoryHub::MemoryHub()
{;}

MemoryHub::~MemoryHub()
{
	free_resource();
}

int MemoryHub::alloc_memory(std::vector<memory_config> &config)
{
	for (auto i : config) {
		Memory *new_memory = new Memory;
		if (!new_memory)
			return -ENOMEM;

		new_memory->alloc_memory(i.gpa_start,
					 i.size, AKVM_MEMORY_SLOT_ALIGN);
		m_memory.push_back(new_memory);
	}
	return 0;
}

bool MemoryHub::find_memory(gpa start, size_t size,
			    std::vector<Imemory*> &result)
{
	bool r = false;

	for (auto i : m_memory) {
		if (start < i->gpa_start())
			continue;
		if (start + size >= i->gpa_start() + i->size())
			continue;
		if (!r) {
			r = true;
			result.clear();
		};
		result.push_back(i);
	}

	return r;
}

void MemoryHub::free_resource(void)
{
	if (m_memory.empty())
		return;

	for (auto i : m_memory)
		delete i;
	m_memory.clear();
}
