#include <cstddef>
#include <cstdint>
#include <stdio.h>
#include <stdlib.h>

int main()
{
	printf("What is your name?\n");

	char buffer[128];

	fgets(buffer, 128, stdin);

	printf("Your name is %s\n", buffer);

	return 0;
}
