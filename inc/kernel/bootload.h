#pragma once

typedef __attribute__((sysv_abi)) void (*BootPrintFunction)(const char16_t* message);

struct KernelBootData
{
	BootPrintFunction Print;
	uint32_t* Framebuffer;
	unsigned int FramebufferWidth;
	unsigned int FramebufferHeight;
	unsigned int FramebufferPitch;

	unsigned int Sum;
};

extern "C"
{
	typedef void __attribute__((sysv_abi)) /*__attribute__((__noreturn__))*/ (*KernelStartFunction)(KernelBootData* bootData);
}
