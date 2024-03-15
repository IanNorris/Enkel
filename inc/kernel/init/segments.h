#pragma once

extern "C" KERNEL_API uint64_t GetGSBase();

struct EnvironmentShared
{
	uint64_t FSBase; //[gs:0]
};

struct EnvironmentKernel : EnvironmentShared
{

};

struct EnvironmentUser : EnvironmentShared
{

};
