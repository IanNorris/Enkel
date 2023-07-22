#pragma once

typedef KERNEL_API void (*BootPrintFunction)(const char16_t* message);

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
	typedef void KERNEL_API /*__attribute__((__noreturn__))*/ (*KernelStartFunction)(KernelBootData* bootData);
}
