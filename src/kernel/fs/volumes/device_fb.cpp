#include "common/types.h"
#include "common/string.h"
#include "memory/virtual.h"
#include "memory/memory.h"
#include "fs/volume.h"
#include "kernel/console/console.h"
#include "kernel/framebuffer/framebuffer.h"
#include "kernel/init/bootload.h"
#include "errno.h"

#include <sys/ioctl.h>
#include <linux/fb.h>

int FrameBufferWidth = 640 * 4;
int FrameBufferHeight = 480 * 4;
int FrameBufferBytesPerPixel = 4;
int FrameBufferByteSize = 0;
void* FrameBufferMemory = 0;

extern KernelBootData GBootData;

VolumeOpenHandleType FramebufferVolume_OpenHandle =
[](VolumeFileHandle volumeHandle, void* context, const char16_t* path, uint8_t mode)
{
	FileHandleMask FH;
	FH.FileHandle = volumeHandle;
	FH.S.FileHandle = 0;

	FrameBufferByteSize = AlignSize(FrameBufferWidth * FrameBufferHeight * FrameBufferBytesPerPixel, PAGE_SIZE);

	FrameBufferMemory = (void*)VirtualAlloc(FrameBufferByteSize, PrivilegeLevel::User);


	//TODO: Remove me
	VirtualProtect(
		GBootData.Framebuffer.Base, 
		AlignSize(GBootData.Framebuffer.Pitch * GBootData.Framebuffer.Height, PAGE_SIZE),
		MemoryProtection::ReadWrite,
		PageFlags_None,
		PrivilegeLevel::User);


	return FH.FileHandle;
};

VolumeCloseHandleType FramebufferVolume_CloseHandle =
[](VolumeFileHandle handle, void* context)
{
	VirtualFree(FrameBufferMemory, FrameBufferByteSize);
	FrameBufferMemory = nullptr;
};

VolumeReadType FramebufferVolume_Read =
[](VolumeFileHandle handle, void* context, uint64_t offset, void* buffer, uint64_t size) -> uint64_t
{

	// TODO - Implement MMIO so we don't call Read here, so we're point to the actual framebuffer

	if (size == 0)
	{
		return 0ULL;
	}

	return 0ULL;
};

VolumeWriteType FramebufferVolume_Write =
[](VolumeFileHandle handle, void* context, uint64_t offset, const void* buffer, uint64_t size) -> uint64_t
{

	return size;
};

VolumeGetSizeType FramebufferVolume_GetSize =
[](VolumeFileHandle handle, void* context) -> uint64_t
{
	return -EINVAL;
};

VolumeSeekType FramebufferVolume_Seek =
[](VolumeFileHandle handle, void* context, int64_t offset, SeekMode origin) -> uint64_t
{
	return -EINVAL;
};

VolumeCommandType FramebufferVolume_Command = 
[](VolumeFileHandle handle, void* context, uint64_t command, uint64_t data) -> uint64_t
{
	switch (command)
	{
	case FBIOGET_VSCREENINFO:
		{
			fb_var_screeninfo* info = (fb_var_screeninfo*)data;
			memset(info, 0, sizeof(fb_var_screeninfo));

			info->xres = GBootData.Framebuffer.Width;
			info->yres = GBootData.Framebuffer.Height;
			info->xres_virtual = 640 * 2;
			info->yres_virtual = 480 * 2;
			info->xoffset = GBootData.Framebuffer.Width / 2 - (info->xres_virtual / 2);
			info->yoffset = GBootData.Framebuffer.Height / 2 - (info->yres_virtual / 2);
			info->bits_per_pixel = 32;

			//info->xres = FrameBufferWidth;
			//info->yres = FrameBufferHeight;
			//info->bits_per_pixel = FrameBufferBytesPerPixel * 8;
					
			return 0;
		}

	case FBIOPUT_VSCREENINFO:
		break;

	case FBIOGET_FSCREENINFO:
		{
			fb_fix_screeninfo* info = (fb_fix_screeninfo*)data;
			memset(info, 0, sizeof(fb_fix_screeninfo));
			//info->smem_start = (long unsigned int)FrameBufferMemory;
			//info->smem_len = FrameBufferByteSize;
			//info->type = FB_TYPE_PACKED_PIXELS;
			//info->visual = FB_VISUAL_DIRECTCOLOR;
			//info->line_length = FrameBufferWidth * FrameBufferBytesPerPixel;
			//info->capabilities = 0;

			info->smem_start = (long unsigned int)GBootData.Framebuffer.Base;
			info->mmio_start = (long unsigned int)GBootData.Framebuffer.Base;
			info->smem_len = GBootData.Framebuffer.Pitch * GBootData.Framebuffer.Height;
			info->mmio_len = GBootData.Framebuffer.Pitch * GBootData.Framebuffer.Height;
			info->type = FB_TYPE_PACKED_PIXELS;
			info->visual = FB_VISUAL_DIRECTCOLOR;
			info->line_length = GBootData.Framebuffer.Pitch;

			return 0;
		}

	case FBIOGETCMAP:
		break;

	case FBIOPUTCMAP:
		break;

	case FBIOPAN_DISPLAY:
		break;

	case FBIO_CURSOR:
		break;

	case FBIOGET_CON2FBMAP:
		break;

	case FBIOPUT_CON2FBMAP:
		break;

	case FBIOBLANK:
		break;

	case FBIOGET_VBLANK:
		break;

	case FBIO_ALLOC:
		break;

	case FBIO_FREE:
		break;

	case FBIOGET_GLYPH:
		break;

	case FBIOGET_HWCINFO:
		break;

	case FBIOPUT_MODEINFO:
		break;

	case FBIOGET_DISPINFO:
		break;

	case FBIO_WAITFORVSYNC:
		break;
	}

	return -EINVAL;
};

Volume FramebufferVolume
{
	OpenHandle: FramebufferVolume_OpenHandle,
	CloseHandle : FramebufferVolume_CloseHandle,
	Read : FramebufferVolume_Read,
	Write : FramebufferVolume_Write,
	GetSize : FramebufferVolume_GetSize,
	Seek : FramebufferVolume_Seek,
	Command: FramebufferVolume_Command,
};
