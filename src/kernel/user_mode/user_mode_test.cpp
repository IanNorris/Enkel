#include "utilities/termination.h"
#include "kernel/console/console.h"

#include "../../apps/hello_world/hello_world.bin.h"

void RunElf(const uint8_t* elfStart);

void EnterUserModeTest()
{
	RunElf((const uint8_t*)hello_world_a_data);
}
