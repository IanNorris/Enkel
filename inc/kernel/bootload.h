#pragma once

extern "C"
{
typedef void (*BootPrintFunction)(const char16_t* message);

struct __attribute__((packed)) KernelBootData
{
	BootPrintFunction Print;
	uint32_t* Framebuffer;
	unsigned int FramebufferWidth;
	unsigned int FramebufferHeight;
	unsigned int FramebufferPitch;

	unsigned int Sum;
};

typedef void __attribute__((__cdecl__)) /*__attribute__((__noreturn__))*/ (*KernelStartFunction)(KernelBootData* bootData);
}