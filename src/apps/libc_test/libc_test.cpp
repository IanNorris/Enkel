#include <cstddef>
#include <cstdint>
#include <stdio.h>
#include <stdlib.h>

volatile thread_local uint64_t TLSThing0 = 0xaaaaaaaaaaaaaaaa;

int main()
{
	printf("Hello World!\n");

	if(TLSThing0 == 0xaaaaaaaaaaaaaaaa)
	{
		printf("TLS test data aligned!\n");
		exit(12345);
	}

	return 0;
}
