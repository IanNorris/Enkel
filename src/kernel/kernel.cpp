#include "utilities/termination.h"
#include "kernel/console/console.h"
#include "kernel/devices/keyboard.h"
#include "kernel/init/apic.h"
#include "kernel/init/bootload.h"
#include "kernel/init/init.h"
#include "kernel/init/pic.h"
#include "kernel/init/tls.h"
#include "kernel/init/msr.h"
#include "kernel/init/long_mode.h"
#include "kernel/init/interrupts.h"
#include "kernel/texture/render.h"
#include "memory/memory.h"
#include "memory/virtual.h"
#include "rpmalloc.h"
#include "../../assets/SplashLogo.h"
#include "kernel/init/acpi.h"
#include "kernel/user_mode/syscall.h"
#include "kernel/user_mode/elf.h"

#include "Protocol/DevicePath.h"
#include "kernel/devices/ahci/cdrom.h"

#include "fs/fs.h"
#include "fs/volume.h"
#include "fs/volumes/stdio.h"
#include "fs/volumes/proc.h"

#include <ff.h>

void InitializeSyscalls();

KernelBootData GBootData;

extern const char16_t* KernelBuildId;

#include "common/string.h"

//Define what a constructor is
typedef void (*StaticInitFunction)();

int sys_execve(const char* filename, const char* const argv[], const char* const envp[]);

extern "C" StaticInitFunction CRTStaticInitStart;
extern "C" StaticInitFunction CRTStaticInitEnd;
extern "C" void CRTInit()
{
    for(StaticInitFunction* InitFunction = &CRTStaticInitStart; InitFunction != &CRTStaticInitEnd; InitFunction++)
	{
		(*InitFunction)();
	}
}

void FinalizeRuntimeServices()
{
	uint32_t runtimeEntries = 0;
	for (uint32_t entry = 0; entry < GBootData.MemoryLayout.Entries; entry++)
    {
        EFI_MEMORY_DESCRIPTOR& Desc = *((EFI_MEMORY_DESCRIPTOR*)((UINT8*)GBootData.MemoryLayout.Map + (entry * GBootData.MemoryLayout.DescriptorSize)));
		if(Desc.Attribute & EFI_MEMORY_RUNTIME)
		{
			runtimeEntries++;
		}
	}

	EFI_MEMORY_DESCRIPTOR* entries = (EFI_MEMORY_DESCRIPTOR*)rpmalloc(sizeof(EFI_MEMORY_DESCRIPTOR) * runtimeEntries);

	int runtimeEntry = 0;
	for (uint32_t entry = 0; entry < GBootData.MemoryLayout.Entries; entry++)
    {
        EFI_MEMORY_DESCRIPTOR& Desc = *((EFI_MEMORY_DESCRIPTOR*)((UINT8*)GBootData.MemoryLayout.Map + (entry * GBootData.MemoryLayout.DescriptorSize)));
	
		if(Desc.Attribute & EFI_MEMORY_RUNTIME)
		{
			entries[runtimeEntry] = Desc;
			runtimeEntry++;
		}
    }

	EFI_STATUS runtimeServicesAddressMapStatus = GBootData.RuntimeServices->SetVirtualAddressMap(sizeof(EFI_MEMORY_DESCRIPTOR) * runtimeEntries, sizeof(EFI_MEMORY_DESCRIPTOR), EFI_MEMORY_DESCRIPTOR_VERSION, entries);
	if(runtimeServicesAddressMapStatus != EFI_SUCCESS)
	{
		char16_t Buffer[16];
		ConsolePrint(u"SetVirtualAddressMap error: 0x");
		witoabuf(Buffer, (int)runtimeServicesAddressMapStatus, 16);
		ConsolePrint(Buffer);
		ConsolePrint(u"\r\n");

		_ASSERTF(false, "Unable to transition to virtual address map for runtime services.");
	}

	rpfree(entries);
}

FRESULT scan_files (
    char16_t* path        /* Start node to be scanned (***also used as work area***) */
)
{
    FRESULT res;
    DIR dir;
    UINT i;
    static FILINFO fno;


    res = f_opendir(&dir, (const TCHAR*)path);                       /* Open the directory */
    if (res == FR_OK) {
        for (;;) {
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
            if (fno.fattrib & AM_DIR) {                    /* It is a directory */
                i = strlen(path);
				strcat(&path[i], u"/");
				strcat(&path[i+1], (const char16_t*)fno.fname);
                //sprintf(&path[i], "/%s", fno.fname);
                res = scan_files(path);                    /* Enter the directory */
                if (res != FR_OK) break;
                path[i] = 0;
            } else {                                       /* It is a file. */
				ConsolePrint(path);
				ConsolePrint(u"/");
				ConsolePrint((const char16_t*)fno.fname);
                ConsolePrint(u"\n");
            }
        }
        f_closedir(&dir);
    }

    return res;
}

extern "C" void __attribute__((sysv_abi, __noreturn__)) KernelMain(KernelBootData * BootData)
{
	OnKernelMainHook();

	GBootData = *BootData;

	DefaultConsoleInit(GBootData.Framebuffer, BMFontColor{ 0x10, 0x20, 0x40 }, BMFontColor{ 0xDF, 0xDF, 0xDF });

	RenderTGA(&GBootData.Framebuffer, SplashLogo_tga_data, GBootData.Framebuffer.Width, 0, AlignImage::Right, AlignImage::Top, true);

	ConsolePrint(u"Enkel ");
	ConsolePrint(KernelBuildId);
	ConsolePrint(u"\n");

	if (IsDebuggerPresent())
	{
		VerboseLog(u"DEBUGGER ATTACHED!\n");
	}

	InitCpuExtensions();

	InitVirtualMemory(&GBootData);

	CRTInit();

	InitInterrupts(&GBootData);

	//FinalizeRuntimeServices();

	InitPIC();

	InitApic(GBootData.Rsdt, GBootData.Xsdt);

	InitKeyboard();

	InitializeSyscalls();
	InitializeAcpica();

	//WalkAcpiTree();

	VerboseLog(u"Mounting filesystem...\n");
	InitializeVolumeSystem();
	InitializeStdioVolumes();
	InitializeSpecialProcVolumes();

	EFI_DEV_PATH* devicePath = (EFI_DEV_PATH*)BootData->BootDevicePath;

	SataBus* sataBus = (SataBus*)rpmalloc(sizeof(SataBus));
	sataBus->Initialize(devicePath);

	CdRomDevice* cdromDevice = (CdRomDevice*)rpmalloc(sizeof(CdRomDevice));
	cdromDevice->Initialize(devicePath, sataBus);

	MountFatVolume(u"/", cdromDevice->GetVolumeId());

	InitializeUserMode();

	VerboseLog(u"Kernel booted.\n\n");

	const char16_t* envp[] = { u"LD_WARN=1", nullptr};

	const char16_t* argvStdio[] = { u"/bash", nullptr };
	ConsolePrint(u"\x25B6 bash\n");
	RunProgram(u"/bash", argvStdio, envp);

	/*
	const char* argvBash[] = { "/busybox", "ash", nullptr};
	const char* envBash[] = { "LD_WARN=1", nullptr};

	sys_execve("/busybox", argvBash, envBash);

	//const char16_t* argv1[] = { u"/hello_world", u"--mode", u"awesome", nullptr};
	//ConsolePrint(u"Running hello_world\n");
	//RunProgram(u"/hello_world", argv1, envp);

	//const char16_t* argv2[] = { u"/tls_test", u"--mode", u"awesome", nullptr };
	//ConsolePrint(u"\nRunning  tls_test\n");
	//RunProgram(u"/tls_test", argv2, envp);

	const char16_t* argv3[] = { u"/libc_test", u"--mode", u"awesome", nullptr };
	ConsolePrint(u"\n> libc_test\n");
	RunProgram(u"/libc_test", argv3, envp);


	const char16_t* argv4[] = { u"/echo", u"This is a test", nullptr };
	ConsolePrint(u"\n#> echo This is a test\n");
	RunProgram(u"/echo", argv4, envp);*/

	/*ConsolePrint(u"\nRunning  static_libc_test\n");
	RunProgram(u"/static_libc_test");

	ConsolePrint(u"\n#> libc_test\n");
	RunProgram(u"libc_test");

	ConsolePrint(u"\n#>\n");*/

	constexpr int bufferSize = 1024;
	char buffer[bufferSize];
	char16_t bufferWide[bufferSize];
	int cursorPos = 0;

	ClearInput();
	ConsolePrint(u"\x25B6 ");

	while (true)
	{
		size_t read = ReadInputBlocking(buffer + cursorPos, bufferSize);
		buffer[read + cursorPos] = 0;

		cursorPos += read;

		if (buffer[cursorPos - 1] == '\n')
		{
			buffer[cursorPos - 1] = 0;

			ascii_to_wide(bufferWide, buffer, bufferSize);

			const char16_t* argv1[] = { bufferWide, nullptr };
			RunProgram(bufferWide, argv1, envp);

			cursorPos = 0;
			ClearInput();
			ConsolePrint(u"\x25B6 ");
		}
	}

	VerboseLog(u"Halted.\n");
	
	while(true)
	{
		asm volatile("hlt");
	}
}
