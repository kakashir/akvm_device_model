#ifndef _MEMORY_HUB_HPP
#define _MEMORY_HUB_HPP
#include "common.hpp"
#include <vector>

struct memory_config {
	gpa gpa_start;
	size_t size;
};

class Imemory;
class MemoryHub {
public:
	/* not MT-safe when first time access */
	static MemoryHub& instance(void)
	{
		static MemoryHub *obj;

		if (!obj)
			obj = new MemoryHub;
		return *obj;
	}
public:
	int alloc_memory(std::vector<memory_config> &config);
	bool find_memory(gpa start, size_t size,
			 std::vector<Imemory*> &result);

	template<typename T>
	void for_each(T &&handler)
	{
		for (auto i : m_memory)
			handler(i);
	}

private:
	MemoryHub();
	~MemoryHub();
	void free_resource(void);

private:
	std::vector<Imemory*> m_memory;
};

#define get_memoryhub() MemoryHub::instance()

#endif
