#pragma once

// https://blog.rchapman.org/posts/Linux_System_Call_Table_for_x86_64/

enum class SyscallIndex
{
	Read, //0
	Write, //1

	NotImplemented2,
	NotImplemented3,
	NotImplemented4,
	NotImplemented5,
	NotImplemented6,
	NotImplemented7,
	NotImplemented8,
	NotImplemented9,

	NotImplemented10,
	NotImplemented11,
	NotImplemented12,
	NotImplemented13,
	NotImplemented14,
	NotImplemented15,
	NotImplemented16,
	NotImplemented17,
	NotImplemented18,
	NotImplemented19,

	NotImplemented20,
	NotImplemented21,
	NotImplemented22,
	NotImplemented23,
	NotImplemented24,
	NotImplemented25,
	NotImplemented26,
	NotImplemented27,
	NotImplemented28,
	NotImplemented29,

	NotImplemented30,
	NotImplemented31,
	NotImplemented32,
	NotImplemented33,
	NotImplemented34,
	NotImplemented35,
	NotImplemented36,
	NotImplemented37,
	NotImplemented38,
	NotImplemented39,

	NotImplemented40,
	NotImplemented41,
	NotImplemented42,
	NotImplemented43,
	NotImplemented44,
	NotImplemented45,
	NotImplemented46,
	NotImplemented47,
	NotImplemented48,
	NotImplemented49,

	NotImplemented50,
	NotImplemented51,
	NotImplemented52,
	NotImplemented53,
	NotImplemented54,
	NotImplemented55,
	NotImplemented56,
	NotImplemented57,
	NotImplemented58,
	NotImplemented59,

	Exit, //60

	Max,
};

struct KernelState
{
	uint64_t RBP;
	uint64_t RSP;
	uint64_t TaskFunction;
};
