#include "kernel/init/tls.h"
#include "memory/memory.h"
#include "kernel/memory/state.h"
#include "memory/virtual.h"
#include "rpmalloc.h"

extern "C" void* __tdata_start;
extern "C" void* __tdata_end;
extern "C" void* __tbss_start;
extern "C" void* __tbss_end;

void InitializeTLS(bool userMode)
{
    size_t tdata_size = (uint8_t*)&__tdata_end - (uint8_t*)&__tdata_start;
    size_t tbss_size = (uint8_t*)&__tbss_end - (uint8_t*)&__tbss_start;
    size_t tls_size = tdata_size + tbss_size;

    // Allocate memory for the TLS.
    uint8_t* tls = (uint8_t*)VirtualAlloc((tls_size + PAGE_SIZE) & ~(PAGE_SIZE-1), userMode ? PrivilegeLevel::User : PrivilegeLevel::Kernel);

    // Initialize the .tdata section.
    memcpy(tls, &__tdata_start, tdata_size);

    // Initialize the .tbss section to zero.
    memset(tls + tdata_size, 0, tbss_size);

    //SetTLSBase(tls + tls_size);

	uint8_t* tlsHigh = tls + tls_size;

	asm volatile("wrmsr" : : "c"(0xC0000100), "a"(tlsHigh), "d"(0));
}
