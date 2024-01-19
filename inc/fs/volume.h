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

typedef VolumeFileHandle (*VolumeOpenHandle)(const char16_t* path, uint8_t mode);
typedef void (*VolumeCloseHandle)(VolumeFileHandle handle);
typedef uint64_t (*VolumeRead)(VolumeFileHandle handle, void* buffer, uint64_t size);
typedef uint64_t (*VolumeWrite)(VolumeFileHandle handle, const void* buffer, uint64_t size);
typedef uint64_t (*VolumeSeek)(VolumeFileHandle handle, uint64_t offset, uint8_t mode);
typedef uint64_t (*VolumeTell)(VolumeFileHandle handle);
typedef uint64_t (*VolumeGetSize)(VolumeFileHandle handle);

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
	VolumeOpenHandle OpenHandle;
	VolumeCloseHandle CloseHandle;
	VolumeRead Read;
	VolumeWrite Write;
	VolumeSeek Seek;
	VolumeTell Tell;
	VolumeGetSize GetSize;
};

// A volume index is a mapping from a mount
// to a supporting volume. The root hash is
// calculated by incrementally adding path
// segments until a volume is found.
// Eg /
//    /mount1
//    /dev1/thing
//    /dev1/thing/abc
struct VolumeIndex
{
	PathHash RootHash;
	const Volume* VolumeImplementation;
} PACKED_ALIGNMENT;

struct FileHandleMask
{
	union
	{
		uint64_t FileHandle;

		struct
		{
			uint64_t FileHandle : 42;
			uint64_t VolumePageIndex: 12;
			uint64_t VolumeIndex : 7;
			uint64_t Segments : 3;
		} PACKED_ALIGNMENT S;
	};
} PACKED_ALIGNMENT;

#define VOLUMES_PER_PAGE ((PAGE_SIZE / sizeof(VolumeIndex))-1) //255
#define MAX_SEGMENTS 8

#define MAX_RESERVED_FILE_HANDLE 1024

void InitializeVolumeSystem();
MountPointHash VolumeHashPath(const char16_t* path);
VolumeHandle MountVolume(const Volume* volume, const char16_t* rootPath);
void UnmountVolume(VolumeHandle handle);

//Pass in a path like /dev1/thing/abc and get
//back the supporting volume, and any path remaining
VolumeHandle BreakPath(const char16_t*& pathInOut);
