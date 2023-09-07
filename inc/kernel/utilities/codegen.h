#pragma once

#include <stdint.h>
#include <stddef.h>

enum class Register
{
	rax,
	rbx,
	rcx,
	rdx,
	rbp,
	rsp,
	rsi,
	rdi,
	r8,
	r9,
	r10,
	r11,
	r12,
	r13,
	r14,
	r15,

	MAX,
};

uint8_t* Write_MovRelativeToRegister(uint8_t* Destination, Register Register, uint8_t* LoadFrom);
