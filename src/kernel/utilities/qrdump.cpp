#include "common/types.h"
#include "utilities/termination.h"
#include "kernel/texture/render.h"
#include "qrcodegen.h"
#include "kernel/utilities/base45.h"
#include "zlib.h"
#include "kernel/init/bootload.h"
#include "common/string.h"

extern KernelBootData GBootData;

struct ZlibBuffer
{
	char TempBuffer[512*1024];
	char* TempBufferPtr;
};

static ZlibBuffer zlibBuffer;

static void *QRZlibAlloc(void *opaque, unsigned int items, unsigned int size)
{
    ZlibBuffer* buffer = (ZlibBuffer*)opaque;

	char* ptr = buffer->TempBufferPtr;

	buffer->TempBufferPtr += (items * size);

	return ptr;
}

static void QRZlibFree(void *opaque, void *address)
{
    //
}

static void CompressData(const unsigned char* Input, unsigned int InputSize, unsigned char* Output, unsigned int* OutputSize)
{
	zlibBuffer.TempBufferPtr = zlibBuffer.TempBuffer;

    z_stream zStream;
    zStream.zalloc = QRZlibAlloc;
    zStream.zfree = QRZlibFree;
    zStream.opaque = &zlibBuffer;

    int ret = deflateInit(&zStream, Z_BEST_COMPRESSION);
    _ASSERTFV(ret == Z_OK, "Zlib deflateInit failed", ret, 0, 0);

    zStream.avail_in = InputSize;
    zStream.next_in = (Bytef*)Input;
    zStream.avail_out = *OutputSize;
    zStream.next_out = Output;

    ret = deflate(&zStream, Z_FINISH);
	_ASSERTFV(ret == Z_STREAM_END, "Zlib deflate failed", ret, 0, 0);
    if (ret != Z_STREAM_END)
	{
        (void)deflateEnd(&zStream);
        return; // Handle error
    }

    *OutputSize = zStream.total_out;
    deflateEnd(&zStream);
}

void QRDump(const char* Data)
{
	char Encoded[10000];

	unsigned char Output[10000];
	unsigned int OutputSize = 10000;

	CompressData((const unsigned char*)Data, strlen(Data), Output, &OutputSize);

	int written = Base45_encode(Encoded, 10*1000, (const char*)Output, OutputSize);
	Encoded[written] = '\0';

	//Use 9 for Surface Go
	RenderQR(&GBootData.Framebuffer, Encoded, 200, 50, 7, true);
}
