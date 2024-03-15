#pragma once

// https://blog.rchapman.org/posts/Linux_System_Call_Table_for_x86_64/

enum class SyscallIndex
{
	Read, //0
	Write, //1
	Open, //2
	Close, //3

	NotImplemented4,
	NotImplemented5,
	NotImplemented6,
	NotImplemented7,
	NotImplemented8,
	MemoryMap, //9

	mprotect, //10
	NotImplemented11,
	Break,
	NotImplemented13,
	NotImplemented14,
	NotImplemented15,
	NotImplemented16,
	sys_pread64, //17
	NotImplemented18,
	NotImplemented19,

	WriteV,
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
	uname, //63
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

	NotImplemented200,
	NotImplemented201,
	NotImplemented202,
	NotImplemented203,
	NotImplemented204,
	NotImplemented205,
	NotImplemented206,
	NotImplemented207,
	NotImplemented208,
	NotImplemented209,
	NotImplemented210,
	NotImplemented211,
	NotImplemented212,
	NotImplemented213,
	NotImplemented214,
	NotImplemented215,
	NotImplemented216,
	NotImplemented217,
	sys_set_tid_address, //218
	NotImplemented219,
	NotImplemented220,
	NotImplemented221,
	NotImplemented222,
	NotImplemented223,
	NotImplemented224,
	NotImplemented225,
	NotImplemented226,
	NotImplemented227,
	NotImplemented228,
	NotImplemented229,
	NotImplemented230,
	sys_exit_group, //231
	NotImplemented232,
	NotImplemented233,
	NotImplemented234,
	NotImplemented235,
	NotImplemented236,
	NotImplemented237,
	NotImplemented238,
	NotImplemented239,
	NotImplemented240,
	NotImplemented241,
	NotImplemented242,
	NotImplemented243,
	NotImplemented244,
	NotImplemented245,
	NotImplemented246,
	NotImplemented247,
	NotImplemented248,
	NotImplemented249,
	NotImplemented250,
	NotImplemented251,
	NotImplemented252,
	NotImplemented253,
	NotImplemented254,
	NotImplemented255,
	NotImplemented256,
	OpenAt, //257
	NotImplemented258,
	NotImplemented259,
	NotImplemented260,
	NotImplemented261,
	sys_newfstatat, //262
	NotImplemented263,
	NotImplemented264,
	NotImplemented265,
	NotImplemented266,
	NotImplemented267,
	NotImplemented268,
	NotImplemented269,
	NotImplemented270,
	NotImplemented271,
	NotImplemented272,
	NotImplemented273,
	NotImplemented274,
	NotImplemented275,
	NotImplemented276,
	NotImplemented277,
	NotImplemented278,
	NotImplemented279,
	NotImplemented280,
	NotImplemented281,
	NotImplemented282,
	NotImplemented283,
	NotImplemented284,
	NotImplemented285,
	NotImplemented286,
	NotImplemented287,
	NotImplemented288,
	NotImplemented289,
	NotImplemented290,
	NotImplemented291,
	NotImplemented292,
	NotImplemented293,
	NotImplemented294,
	NotImplemented295,
	NotImplemented296,
	NotImplemented297,
	NotImplemented298,
	NotImplemented299,

	NotImplemented300,
	NotImplemented301,
	NotImplemented302,
	NotImplemented303,
	NotImplemented304,
	NotImplemented305,
	NotImplemented306,
	NotImplemented307,
	NotImplemented308,
	NotImplemented309,
	NotImplemented310,
	NotImplemented311,
	NotImplemented312,
	NotImplemented313,
	NotImplemented314,
	NotImplemented315,
	NotImplemented316,
	NotImplemented317,
	NotImplemented318,
	NotImplemented319,
	NotImplemented320,
	NotImplemented321,
	NotImplemented322,
	NotImplemented323,
	NotImplemented324,
	NotImplemented325,
	NotImplemented326,
	NotImplemented327,
	NotImplemented328,
	NotImplemented329,
	NotImplemented330,
	NotImplemented331,
	NotImplemented332,
	NotImplemented333,
	NotImplemented334,
	NotImplemented335,
	NotImplemented336,
	NotImplemented337,
	NotImplemented338,
	NotImplemented339,
	NotImplemented340,
	NotImplemented341,
	NotImplemented342,
	NotImplemented343,
	NotImplemented344,
	NotImplemented345,
	NotImplemented346,
	NotImplemented347,
	NotImplemented348,
	NotImplemented349,
	NotImplemented350,
	NotImplemented351,
	NotImplemented352,
	NotImplemented353,
	NotImplemented354,
	NotImplemented355,
	NotImplemented356,
	NotImplemented357,
	NotImplemented358,
	NotImplemented359,
	NotImplemented360,
	NotImplemented361,
	NotImplemented362,
	NotImplemented363,
	NotImplemented364,
	NotImplemented365,
	NotImplemented366,
	NotImplemented367,
	NotImplemented368,
	NotImplemented369,
	NotImplemented370,
	NotImplemented371,
	NotImplemented372,
	NotImplemented373,
	NotImplemented374,
	NotImplemented375,
	NotImplemented376,
	NotImplemented377,
	NotImplemented378,
	NotImplemented379,
	NotImplemented380,
	NotImplemented381,
	NotImplemented382,
	NotImplemented383,
	NotImplemented384,
	NotImplemented385,
	NotImplemented386,
	NotImplemented387,
	NotImplemented388,
	NotImplemented389,
	NotImplemented390,
	NotImplemented391,
	NotImplemented392,
	NotImplemented393,
	NotImplemented394,
	NotImplemented395,
	NotImplemented396,
	NotImplemented397,
	NotImplemented398,
	NotImplemented399,

	//WARNING: Don't forget to update the hardcoded limit in dispatcher.asm!

	Max,
};

struct KernelState
{
	uint64_t RBP;
	uint64_t RSP;
	uint64_t TaskFunction;
};
