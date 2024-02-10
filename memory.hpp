#ifndef __MEMORY_HPP
#define __MEMORY_HPP

#include "common.hpp"

interface Imemory {
	virtual ~Imemory() {;}
	virtual int alloc_memory(gpa gpa_start, size_t size, int align) = 0;
	virtual gpa gpa_start(void) = 0;
	virtual size_t size(void) = 0;
	virtual hva hva_start(void) = 0;
};

class Memory : public Imemory {
public:
	Memory();
	virtual ~Memory();
public:
	int alloc_memory(gpa gpa_start, size_t size, int align);
	gpa gpa_start(void) { return m_gpa_start; }
	size_t size(void) { return m_size; }
	hva hva_start(void) { return reinterpret_cast<hva>(m_hva); }
private:
	void free_resource(void);
private:
	gpa m_gpa_start;
	void *m_hva;
	size_t m_size;
};

#endif
