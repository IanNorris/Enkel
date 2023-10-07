#include "kernel/console/console.h"
#include "memory/memory.h"
#include "memory/virtual.h"
#include "rpmalloc.h"

#include "rpnew.h"

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
	return VirtualAlloc(size);
}

void RPMallocUnmap(void* address, size_t size, size_t offset, size_t release)
{
	NOT_IMPLEMENTED();
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
	rpmalloc_thread_initialize();
}