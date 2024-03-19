
/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2023        */
/*                                                                       */
/*   Portions COPYRIGHT 2017-2023 STMicroelectronics                     */
/*   Portions Copyright (C) 2013, ChaN, all right reserved               */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various existing      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "common/string.h"
#include "memory/memory.h"
#include "kernel/init/acpi.h"
#include "kernel/devices/ahci/cdrom.h"
#include "kernel/scheduling/time.h"
#include <ff.h>
#include <diskio.h>
#include <rpmalloc.h>

#include <fcntl.h>

#include "fs/volume.h"

extern "C" KERNEL_API uint64_t _rdtsc();

uint64_t GCDPartitionStartSector = 33;

uint64_t GCDRomBufferSize;
uint8_t* GCDRomBuffer;

struct FatDrive
{
	VolumeHandle Volume;
	FATFS FileSystem;
};

FatDrive FatDrives[32];
uint64_t MountedFatDrives = 0;

#define MAX_FILE_HANDLES 512
FIL* FileHandles[MAX_FILE_HANDLES];


extern "C"
{

	/**
	 * @brief  Gets Disk Status
	 * @param  pdrv: Physical drive number (0..)
	 * @retval DSTATUS: Operation status
	 */
	DSTATUS disk_status(BYTE pdrv)
	{
		return RES_OK;
	}

	/**
	 * @brief  Initializes a Drive
	 * @param  pdrv: Physical drive number (0..)
	 * @retval DSTATUS: Operation status
	 */
	DSTATUS disk_initialize(BYTE pdrv)
	{
		return RES_OK;
	}

	/**
	 * @brief  Reads Sector(s)
	 * @param  pdrv: Physical drive number (0..)
	 * @param  *buff: Data buffer to store read data
	 * @param  sector: Sector address (LBA)
	 * @param  count: Number of sectors to read (1..128)
	 * @retval DRESULT: Operation result
	 */
	DRESULT disk_read(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count )
	{
		const UINT sectorSize = 512;
		const UINT cdSectorSize = 2048;

		FatDrive& drive = FatDrives[pdrv];

		uint64_t read = VolumeRead(drive.Volume, (GCDPartitionStartSector * cdSectorSize) + (sector * sectorSize), buff, count * sectorSize);

		//TODO
		/*if(read == count * sectorSize)
		{
			return RES_OK;
		}

		return RES_ERROR;*/
		return RES_OK;
	}

	/**
	 * @brief  Writes Sector(s)
	 * @param  pdrv: Physical drive number (0..)
	 * @param  *buff: Data to be written
	 * @param  sector: Sector address (LBA)
	 * @param  count: Number of sectors to write (1..128)
	 * @retval DRESULT: Operation result
	 */

	DRESULT disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count)
	{
		return RES_WRPRT;
	}

	/**
	 * @brief  I/O control operation
	 * @param  pdrv: Physical drive number (0..)
	 * @param  cmd: Control code
	 * @param  *buff: Buffer to send/receive control data
	 * @retval DRESULT: Operation result
	 */

	DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff)
	{
		if(cmd == GET_SECTOR_SIZE)
		{
			*((uint16_t*)buff) = 512;
			return RES_OK;
		}

		NOT_IMPLEMENTED();
		return RES_PARERR;
	}

	/**
	 * @brief  Gets Time from RTC
	 * @param  None
	 * @retval Time in DWORD
	 */
	DWORD get_fattime(void)
	{
		return _rdtsc();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////

VolumeOpenHandleType FatVolume_OpenHandle = 
[](VolumeFileHandle volumeHandle, void* context, const char16_t* path, uint8_t mode) -> uint64_t
{
	char16_t adjustedPath[256];
	strcpy(adjustedPath, u"/");
	strcat(adjustedPath, path);

	for(int i = 0; i < MAX_FILE_HANDLES; i++)
	{
		if(FileHandles[i] == nullptr)
		{
			FileHandles[i] = (FIL*)rpmalloc(sizeof(FIL));

			FRESULT fr = f_open(FileHandles[i], (const TCHAR*)adjustedPath, FA_READ); //TODO: mode
			if(fr == FR_OK)
			{
				FileHandleMask FH;
				FH.FileHandle = volumeHandle;
				FH.S.FileHandle = i;

				return FH.FileHandle;
			}
			else
			{
				rpfree(FileHandles[i]);
				FileHandles[i] = nullptr;
				return (VolumeFileHandle)0ULL;
			}
		}
	}

	return (VolumeFileHandle)0ULL;
};

VolumeCloseHandleType FatVolume_CloseHandle = 
[](VolumeFileHandle handle, void* context)
{
	FileHandleMask FH;
	FH.FileHandle = handle;

	if(FileHandles[FH.S.FileHandle] != nullptr)
	{
		rpfree(FileHandles[FH.S.FileHandle]);
		FileHandles[FH.S.FileHandle] = nullptr;
	}
};

VolumeReadType FatVolume_Read = 
[](VolumeFileHandle handle, void* context, uint64_t offset, void* buffer, uint64_t size) -> uint64_t
{
	if (size == 0)
	{
		return 0ULL;
	}

	FileHandleMask FH;
	FH.FileHandle = handle;

	UINT bytesRead = 0;

	uint64_t tell = f_tell(FileHandles[FH.S.FileHandle]);

	if (offset != -1)
	{
		if (offset < 0)
		{
			_ASSERTF(false, "Invalid offset");
		}
		else
		{
			f_lseek(FileHandles[FH.S.FileHandle], offset);
		}
	}

	FRESULT fr = f_read(FileHandles[FH.S.FileHandle], buffer, size, &bytesRead);

	if (offset != -1)
	{
		f_lseek(FileHandles[FH.S.FileHandle], tell);
	}

	if(fr != FR_OK)
	{
		return 0;
	}
	
	return bytesRead;
};

VolumeWriteType FatVolume_Write = 
[](VolumeFileHandle handle, void* context, uint64_t offset, const void* buffer, uint64_t size) -> uint64_t
{
	if (size == 0)
	{
		return 0ULL;
	}

	FileHandleMask FH;
	FH.FileHandle = handle;

	UINT bytesWritten = 0;

	uint64_t tell = f_tell(FileHandles[FH.S.FileHandle]);

	if (offset != -1)
	{
		if (offset < 0)
		{
			_ASSERTF(false, "Invalid offset");
		}
		else
		{
			f_lseek(FileHandles[FH.S.FileHandle], offset);
		}
	}

	FRESULT fr = f_write(FileHandles[FH.S.FileHandle], buffer, size, &bytesWritten);

	if (offset != -1)
	{
		f_lseek(FileHandles[FH.S.FileHandle], tell);
	}

	if(fr != FR_OK)
	{
		return 0;
	}
	
	return bytesWritten;
};

VolumeGetSizeType FatVolume_GetSize = 
[](VolumeFileHandle handle, void* context) -> uint64_t
{
	FileHandleMask FH;
	FH.FileHandle = handle;

	uint64_t size = f_size(FileHandles[FH.S.FileHandle]);
	
	return size;
};

Volume FatVolume
{
	OpenHandle: FatVolume_OpenHandle,
	CloseHandle: FatVolume_CloseHandle,
	Read: FatVolume_Read,
	Write: FatVolume_Write,
	GetSize: FatVolume_GetSize
};

VolumeHandle MountFatVolume(const char16_t* mountPoint, VolumeHandle volume)
{
	void* context = &FatDrives[MountedFatDrives];

	char16_t driveIndex[9];
	witoabuf(driveIndex, MountedFatDrives, 10);

	FatDrives[MountedFatDrives].Volume = volume;
    f_mount(&FatDrives[MountedFatDrives].FileSystem, (const TCHAR*)driveIndex, 1);
	
	MountedFatDrives++;

	return MountVolume(&FatVolume, mountPoint, context);
}
