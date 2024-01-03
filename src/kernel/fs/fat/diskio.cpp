
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

extern CdRomDevice* GCDRomDevice;

extern "C" KERNEL_API uint64_t _rdtsc();

uint64_t GCDPartitionStartSector = 33;

uint64_t GCDRomBufferSize;
uint8_t* GCDRomBuffer;

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
		GCDRomBufferSize = 0;
		GCDRomBuffer = nullptr;
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
		UINT startSector = sector & ~((2048/512)-1);
		UINT sectorCountUpperBound = 2 + (count / 4);
		UINT sectorUpperBoundSize = sectorCountUpperBound * 2048;

		if(GCDRomBufferSize < sectorUpperBoundSize)
		{
			if(GCDRomBuffer)
			{
				rpfree(GCDRomBuffer);
			}

			GCDRomBufferSize = sectorUpperBoundSize;
			GCDRomBuffer = (uint8_t*)rpmalloc(sectorUpperBoundSize);
		}

		GCDRomDevice->ReadSectors(GCDPartitionStartSector + (sector / 4), sectorCountUpperBound, GCDRomBuffer);

		UINT sectorOffset = (sector - startSector);
		memcpy(buff, GCDRomBuffer + (sectorOffset * 512), count * 512);

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
		/*DRESULT res;

		res = disk.drv[pdrv]->disk_write(disk.lun[pdrv], buff, sector, count);
		return res;*/

		return RES_OK;
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
			*((uint16_t*)buff) = 2048;
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