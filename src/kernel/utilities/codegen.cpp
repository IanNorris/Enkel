#include "kernel/utilities/codegen.h"
#include "common/types.h"

uint8_t* Write_MovRelativeToRegister(uint8_t* Destination, Register Register, uint8_t* LoadFrom)
{
	switch(Register)
	{
		case Register::rax:
			*Destination++ = 0x48;
			*Destination++ = 0x8B;
			*Destination++ = 0x05;
			break;

		case Register::rbx:
			*Destination++ = 0x48;
			*Destination++ = 0x8B;
			*Destination++ = 0x1D;
			break;

		case Register::rcx:
			*Destination++ = 0x48;
			*Destination++ = 0x8B;
			*Destination++ = 0x0D;
			break;

		case Register::rdx:
			*Destination++ = 0x48;
			*Destination++ = 0x8B;
			*Destination++ = 0x15;
			break;

		case Register::rbp:
			*Destination++ = 0x48;
			*Destination++ = 0x8B;
			*Destination++ = 0x2D;
			break;

		case Register::rsp:
			*Destination++ = 0x48;
			*Destination++ = 0x8B;
			*Destination++ = 0x25;
			break;

		case Register::rsi:
			*Destination++ = 0x48;
			*Destination++ = 0x8B;
			*Destination++ = 0x35;
			break;

		case Register::rdi:
			*Destination++ = 0x48;
			*Destination++ = 0x8B;
			*Destination++ = 0x3D;
			break;

		case Register::r8:
			*Destination++ = 0x4C;
			*Destination++ = 0x8B;
			*Destination++ = 0x05;
			break;

		case Register::r9:
			*Destination++ = 0x4C;
			*Destination++ = 0x8B;
			*Destination++ = 0x0D;
			break;

		case Register::r10:
			*Destination++ = 0x4C;
			*Destination++ = 0x8B;
			*Destination++ = 0x15;
			break;

		case Register::r11:
			*Destination++ = 0x4C;
			*Destination++ = 0x8B;
			*Destination++ = 0x1D;
			break;

		case Register::r12:
			*Destination++ = 0x4C;
			*Destination++ = 0x8B;
			*Destination++ = 0x25;
			break;

		case Register::r13:
			*Destination++ = 0x4C;
			*Destination++ = 0x8B;
			*Destination++ = 0x2D;
			break;

		case Register::r14:
			*Destination++ = 0x4C;
			*Destination++ = 0x8B;
			*Destination++ = 0x35;
			break;

		case Register::r15:
			*Destination++ = 0x4C;
			*Destination++ = 0x8B;
			*Destination++ = 0x3D;
			break;

		default:
			_ASSERTF(false, "Invalid register");
	}

	uint64_t OffsetToEnd = (uint64_t)LoadFrom - (uint64_t)(Destination+sizeof(uint32_t));
	*(uint32_t*)Destination = (uint32_t)(OffsetToEnd);
	Destination += sizeof(uint32_t);

	return Destination;
}
