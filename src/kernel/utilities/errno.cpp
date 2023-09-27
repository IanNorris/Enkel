#include "kernel/init/tls.h"

volatile __thread int __errno_value = 0;

extern "C"
{
	volatile int* __errno_location(void)
	{
		return &__errno_value;
	}
}
