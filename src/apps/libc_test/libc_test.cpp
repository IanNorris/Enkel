#include <cstddef>
#include <cstdint>
#include <stdio.h>
#include <stdlib.h>

int main()
{
	//Can't use static libraries as it crashes because it's attempting to relocate to a specific address.

	printf("Hello World!\n");

	return 0;
}
