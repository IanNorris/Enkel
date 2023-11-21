#include "utilities/termination.h"
#include "kernel/console/console.h"
#include "kernel/init/bootload.h"
#include "kernel/init/init.h"
#include "kernel/init/interrupts.h"
#include "memory/memory.h"
#include "memory/virtual.h"
#include "rpmalloc.h"
#include "kernel/init/acpi.h"
#include "common/string.h"

/*
int ReadFromAHCIPort(HBAPort *port, uint64_t startl, uint64_t starth, uint32_t count, uint8_t *buf)
{
	// Ensure the command list and FIS are running
	port->cmd &= ~(HBA_PxCMD_ST | HBA_PxCMD_FRE);
	while (port->cmd & (HBA_PxCMD_CR | HBA_PxCMD_FR))
		;

	port->cmd |= HBA_PxCMD_FRE;
	port->cmd |= HBA_PxCMD_ST;

	// Obtain a command slot
	int slot = FindCommandSlot(port);
	if (slot == -1)
		return -1; // No available slot

	HBA_CMD_HEADER *cmdheader = (HBA_CMD_HEADER *)(port->clb);
	cmdheader += slot;
	cmdheader->flags = 0x05; // FIS length 5 words, write, prefetchable
	cmdheader->prdtl = 1;	 // 1 PRDT entry

	HBA_CMD_TBL *cmdtbl = (HBA_CMD_TBL *)(cmdheader->ctba);
	memset(cmdtbl, 0, sizeof(HBA_CMD_TBL) + (cmdheader->prdtl - 1) * sizeof(HBA_PRDT_ENTRY));

	// Setup PRDT
	cmdtbl->prdt_entry[0].dba = (uint32_t)buf;
	cmdtbl->prdt_entry[0].dbc = count * 2048 - 1; // 2048 bytes per sector
	cmdtbl->prdt_entry[0].i = 1;				  // Interrupt on completion

	// Setup command
	FIS_REG_H2D *cmdfis = (FIS_REG_H2D *)(&cmdtbl->cfis);
	cmdfis->fis_type = FIS_TYPE_REG_H2D;
	cmdfis->c = 1; // Command
	cmdfis->command = ATA_CMD_READ_DMA_EX;
	cmdfis->lba0 = (uint8_t)startl;
	cmdfis->lba1 = (uint8_t)(startl >> 8);
	// Set other LBA and device fields as required
	cmdfis->countl = count & 0xFF;
	cmdfis->counth = (count >> 8) & 0xFF;

	// Issue command
	port->ci = 1 << slot; // Issue command

	// Wait for completion
	while (1)
	{
		// Check if command completed
		if ((port->ci & (1 << slot)) == 0)
			break;
		if (port->is & HBA_PxIS_TFES)
		{
			// Task file error
			return -1;
		}
	}

	// Check again for task file error
	if (port->is & HBA_PxIS_TFES)
	{
		return -1;
	}

	return count * 2048; // Return number of bytes read
}

int FindCommandSlot(HBAPort *port)
{
	// Find a free command list slot
	uint32_t slots = (port->sact | port->ci);
	for (int i = 0; i < 32; i++)
	{
		if ((slots & 1) == 0)
			return i;
		slots >>= 1;
	}
	return -1;
}
*/
