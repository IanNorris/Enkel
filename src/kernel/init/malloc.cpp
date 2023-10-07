#include "kernel/console/console.h"
#include "memory/memory.h"
#include "memory/virtual.h"
#include "rpmalloc.h"

#include "rpnew.h"

#define pointer_offset(ptr, ofs) (void*)((char*)(ptr) + (ptrdiff_t)(ofs))

extern "C"
{
	extern size_t _memory_page_size;
	extern size_t _memory_map_granularity;
	extern size_t _memory_span_size;
	extern uintptr_t _memory_span_mask;
}

void OnRPMallocError(const char* message)
{
	char16_t Buffer[256];
	char16_t* BufferPtr = Buffer;
	while((*BufferPtr++ = *message) != 0 && (BufferPtr - Buffer < 255));

	*BufferPtr = '\0';

	ConsolePrint(Buffer);

	_ASSERTF(false, "RPMalloc failed");
}

void* RPMallocMap(size_t size, size_t* offset)
{
	//Either size is a heap (a single page) or a (multiple) span - we only need to align spans, and only if larger than map granularity
	size_t padding = ((size >= _memory_span_size) && (_memory_span_size > _memory_map_granularity)) ? _memory_span_size : 0;
	_ASSERTF(size >= _memory_page_size, "Invalid mmap size");

	void* ptr = VirtualAlloc(size + padding);
	if (!ptr) {
		_ASSERTF(ptr, "Failed to map virtual memory block");
		return 0;
	}

	//_rpmalloc_stat_add(&_mapped_pages_os, (int32_t)((size + padding) >> _memory_page_size_shift));
	if (padding) {
		size_t final_padding = padding - ((uintptr_t)ptr & ~_memory_span_mask);
		_ASSERTF(final_padding <= _memory_span_size, "Internal failure in padding");
		_ASSERTF(final_padding <= padding, "Internal failure in padding");
		_ASSERTF(!(final_padding % 8), "Internal failure in padding");
		ptr = pointer_offset(ptr, final_padding);
		*offset = final_padding >> 3;
	}
	_ASSERTF((size < _memory_span_size) || !((uintptr_t)ptr & ~_memory_span_mask), "Internal failure in padding");
	return ptr;
}

//! Default implementation to unmap pages from virtual memory
void RPMallocUnmap(void* address, size_t size, size_t offset, size_t release)
{
	_ASSERTF(release || (offset == 0), "Invalid unmap size");
	_ASSERTF(!release || (release >= _memory_page_size), "Invalid unmap size");
	_ASSERTF(size >= _memory_page_size, "Invalid unmap size");
	if (release && offset) {
		offset <<= 3;
		address = pointer_offset(address, -(int32_t)offset);
		if ((release >= _memory_span_size) && (_memory_span_size > _memory_map_granularity)) {
			//Padding is always one span size
			release += _memory_span_size;
		}
	}

	if (!VirtualFree(address, release ? 0 : size))
	{
		_ASSERTF(0, "Failed to unmap virtual memory block");
	}

	if (release)
	{
		//_rpmalloc_stat_sub(&_mapped_pages_os, release >> _memory_page_size_shift);
	}
}

void InitRPMalloc()
{
	rpmalloc_config_t rpmConfig;
	memset(&rpmConfig, 0, sizeof(rpmConfig));

	rpmConfig.memory_map = &RPMallocMap;
	rpmConfig.memory_unmap = &RPMallocUnmap;
	rpmConfig.error_callback = &OnRPMallocError;
	rpmConfig.page_size = 4096;

	rpmalloc_initialize_config(&rpmConfig);
}