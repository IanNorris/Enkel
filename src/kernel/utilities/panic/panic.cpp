#include "common/types.h"
#include "utilities/termination.h"
#include "utilities/qrdump.h"
#include "kernel/panic/panic_qr.h"
#include "common/string.h"
#include "memory/memory.h"

extern "C" void GetPanicCPUState(Panic::CPURegs* regs);

void DumpLiveCPUState()
{
	char buffer[16 * 1024];
	char* bufferPointer = buffer;

	Panic::CPURegs regs;
	memset(&regs, 0, sizeof(regs));

	GetPanicCPUState(&regs);

	Panic::Header header;
	header.MagicByte = QR_MAGIC_BYTE;
	header.Version = QR_VERSION;
	header.Page = 0;
	header.PageCount = 1;
	header.Pad = QR_HEADER_PAD;

	memcpy(bufferPointer, &header, sizeof(header)); bufferPointer += sizeof(header);

	Panic::PacketHeader packetHeader;
	memset(&packetHeader, 0, sizeof(packetHeader));
	packetHeader.Type = QR_PACKET_TYPE_CPU_REGS;
	packetHeader.Size = sizeof(regs);
	memcpy(bufferPointer, &packetHeader, sizeof(packetHeader)); bufferPointer += sizeof(packetHeader);

	memcpy(bufferPointer, &regs, sizeof(regs)); bufferPointer += sizeof(regs);

	const int contrast = 2;

	QRDumpBytes(buffer, bufferPointer-buffer, contrast);
}
