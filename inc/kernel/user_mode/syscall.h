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
	MemoryMap,

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
	sys_access,
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
	NotImplemented61,
	NotImplemented62,
	NotImplemented63,
	NotImplemented64,
	NotImplemented65,
	NotImplemented66,
	NotImplemented67,
	NotImplemented68,
	NotImplemented69,

	NotImplemented70,
	NotImplemented71,
	NotImplemented72,
	NotImplemented73,
	NotImplemented74,
	NotImplemented75,
	NotImplemented76,
	NotImplemented77,
	NotImplemented78,
	NotImplemented79,

	NotImplemented80,
	NotImplemented81,
	NotImplemented82,
	NotImplemented83,
	NotImplemented84,
	NotImplemented85,
	NotImplemented86,
	NotImplemented87,
	NotImplemented88,
	NotImplemented89,

	NotImplemented90,
	NotImplemented91,
	NotImplemented92,
	NotImplemented93,
	NotImplemented94,
	NotImplemented95,
	NotImplemented96,
	NotImplemented97,
	NotImplemented98,
	NotImplemented99,

	NotImplemented100,
	NotImplemented101,
	NotImplemented102,
	NotImplemented103,
	NotImplemented104,
	NotImplemented105,
	NotImplemented106,
	NotImplemented107,
	NotImplemented108,
	NotImplemented109,
	NotImplemented110,
	NotImplemented111,
	NotImplemented112,
	NotImplemented113,
	NotImplemented114,
	NotImplemented115,
	NotImplemented116,
	NotImplemented117,
	NotImplemented118,
	NotImplemented119,
	NotImplemented120,
	NotImplemented121,
	NotImplemented122,
	NotImplemented123,
	NotImplemented124,
	NotImplemented125,
	NotImplemented126,
	NotImplemented127,
	NotImplemented128,
	NotImplemented129,
	NotImplemented130,
	NotImplemented131,
	NotImplemented132,
	NotImplemented133,
	NotImplemented134,
	NotImplemented135,
	NotImplemented136,
	NotImplemented137,
	NotImplemented138,
	NotImplemented139,

	NotImplemented140,
	NotImplemented141,
	NotImplemented142,
	NotImplemented143,
	NotImplemented144,
	NotImplemented145,
	NotImplemented146,
	NotImplemented147,
	NotImplemented148,
	NotImplemented149,
	NotImplemented150,
	NotImplemented151,
	NotImplemented152,
	NotImplemented153,
	NotImplemented154,
	NotImplemented155,
	NotImplemented156,
	NotImplemented157,
	sys_arch_prctl,
	NotImplemented159,
	NotImplemented160,
	NotImplemented161,
	NotImplemented162,
	NotImplemented163,
	NotImplemented164,
	NotImplemented165,
	NotImplemented166,
	NotImplemented167,
	NotImplemented168,
	NotImplemented169,
	NotImplemented170,
	NotImplemented171,
	NotImplemented172,
	NotImplemented173,
	NotImplemented174,
	NotImplemented175,
	NotImplemented176,
	NotImplemented177,
	NotImplemented178,
	NotImplemented179,
	NotImplemented180,
	NotImplemented181,
	NotImplemented182,
	NotImplemented183,
	NotImplemented184,
	NotImplemented185,
	NotImplemented186,
	NotImplemented187,
	NotImplemented188,
	NotImplemented189,
	NotImplemented190,
	NotImplemented191,
	NotImplemented192,
	NotImplemented193,
	NotImplemented194,
	NotImplemented195,
	NotImplemented196,
	NotImplemented197,
	NotImplemented198,
	NotImplemented199,

	Max,
};

struct KernelState
{
	uint64_t RBP;
	uint64_t RSP;
	uint64_t TaskFunction;
};
