#include <cstddef>
#include <cstdint>
#include <stdio.h>
#include <stdlib.h>

volatile thread_local uint64_t TLSThing0 = 0xaaaaaaaaaaaaaaaa;

int main()
{
	printf("Hello World!\n");

	if(TLSThing0 != 0xeeeeeeeeeeeeeeee)
	{
		printf("TLS test data 2 not aligned!\n");
		exit(2);
	}

	return 0;
}
