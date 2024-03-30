#pragma once

extern "C" KERNEL_API uint64_t GetGSBase();

struct EnvironmentShared
{
	uint64_t FSBase; //[gs:0]
};

struct EnvironmentKernel : EnvironmentShared
{
	uint64_t SyscallStack; // [gs:8]
	uint64_t SyscallTable; // [gs:16]
};

struct EnvironmentUser : EnvironmentShared
{

};
