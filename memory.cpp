#include <cstdlib>
#include <cerrno>
#include "memory.hpp"

Memory::Memory(void):m_gpa_start(0ULL),m_size(0ULL),m_hva(NULL)
{}

Memory::~Memory(void)
{
	free_resource();
}

void Memory::free_resource(void)
{
	if (!m_hva)
		return;
	std::free(m_hva);
	m_hva = NULL;
}

int Memory::alloc_memory(gpa gpa_start, size_t size, int align)
{
	free_resource();
	m_hva = std::aligned_alloc(align, size);
	if (!m_hva)
		return -ENOMEM;

	m_gpa_start = gpa_start;
	m_size = size;
	return 0;
}
