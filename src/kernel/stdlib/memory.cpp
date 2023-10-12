#include "rpmalloc.h"

extern "C"
{
	void* malloc(size_t size)
	{
		return rpmalloc(size);
	}

	void* calloc(size_t items, size_t size)
	{
		return rpmalloc(items * size);
	}

	void free(void* ptr)
	{
		rpfree(ptr);
	}
} 
