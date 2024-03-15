#pragma once

enum class FileAccessMode : uint8_t
{
	Read = 1,
	Write = 2,
	Append = 4,
};

typedef uint64_t PathHash;

typedef uint64_t VolumeHandle;
typedef uint64_t FileHandle;
typedef uint64_t VolumeFileHandle;

typedef VolumeFileHandle (*VolumeOpenHandleType)(VolumeFileHandle volumeHandle, void* context, const char16_t* path, uint8_t mode);
typedef void (*VolumeCloseHandleType)(VolumeFileHandle handle, void* context);
typedef uint64_t (*VolumeReadType)(VolumeFileHandle handle, void* context, uint64_t offset,void* buffer, uint64_t size);
typedef uint64_t (*VolumeWriteType)(VolumeFileHandle handle, void* context, uint64_t offset, const void* buffer, uint64_t size);
typedef uint64_t (*VolumeGetSizeType)(VolumeFileHandle handle, void* context);

struct MountPointHash
{
	PathHash Hash;
	uint64_t Segments;
};

// Underlying volume for reading/writing to a "file".
// The "file" can be anything, such as an input device,
// network connection, or a virtual file.
struct Volume
{
	VolumeOpenHandleType OpenHandle;
	VolumeCloseHandleType CloseHandle;
	VolumeReadType Read;
	VolumeWriteType Write;
	VolumeGetSizeType GetSize;
};

// A volume index is a mapping from a mount
// to a supporting volume. The root hash is
// calculated by incrementally adding path
// segments until a volume is found.
// Eg /
//    /mount1
//    /device/1/thing
//    /device/1/thing/abc
struct VolumeIndex
{
	PathHash RootHash;
	const Volume* VolumeImplementation;
	void* Context;
} PACKED_ALIGNMENT;

//glibc only supports 32bit file handles
/*struct FileHandleMask
{
	union
	{
		VolumeFileHandle FileHandle;

		struct
		{
			uint64_t FileHandle : 42;
			uint64_t VolumePageIndex: 12;
			uint64_t VolumeIndex : 7;
			uint64_t Segments : 3;
		} PACKED_ALIGNMENT S;
	};
} PACKED_ALIGNMENT;*/

struct FileHandleMask
{
	union
	{
		VolumeFileHandle FileHandle;

		struct
		{
			uint32_t SignBit : 1;
			uint32_t FileHandle : 21;
			uint32_t VolumePageIndex : 3;
			uint32_t VolumeIndex : 3;
			uint32_t Segments : 3;

		} PACKED_ALIGNMENT S;
	};
} PACKED_ALIGNMENT;

#define VOLUMES_PER_PAGE 170
#define MAX_SEGMENTS 8

#define MAX_RESERVED_FILE_HANDLE 1024

void InitializeVolumeSystem();
MountPointHash VolumeHashPath(const char16_t* path);
VolumeHandle MountVolume(const Volume* volume, const char16_t* rootPath, void* volumeContext);
void UnmountVolume(VolumeHandle handle);

VolumeFileHandle VolumeOpenHandle(VolumeFileHandle volumeHandle, const char16_t* path, uint8_t mode);
void VolumeCloseHandle(VolumeFileHandle handle);
uint64_t VolumeRead(VolumeFileHandle handle, uint64_t offset, void* buffer, uint64_t size);
uint64_t VolumeWrite(VolumeFileHandle handle, uint64_t offset, const void* buffer, uint64_t size);
uint64_t VolumeGetSize(VolumeFileHandle handle);

//Pass in a path like /dev1/thing/abc and get
//back the supporting volume, and any path remaining
VolumeHandle BreakPath(const char16_t*& pathInOut);
