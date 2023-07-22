#pragma once

typedef void (*BootPrintFunction)(const char16_t* message);
typedef void __attribute__((__noreturn__)) (*KernelStartFunction)(BootPrintFunction printFunction);