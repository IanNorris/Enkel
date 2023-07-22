#include "fs/fat/fat.h"
#include "memory/memory.h"
#include "common/string.h"

FILE_SYSTEM_FAT GetFileSystemType(const uint8_t* dataPtr)
{
    bootsector* bootsect = (bootsector*)dataPtr;

    //All three FAT types have the same signature in the same location
    if (bootsect->bs33.bsBootSectSig0 == BOOTSIG0 && bootsect->bs33.bsBootSectSig1 == BOOTSIG1)
    {
        //Can read these variables as they alias in all three FATs.
        bpb33* bpb = (bpb33*)(bootsect->bs33.bsBPB);
        uint32_t clusterCount = bpb->bpbSectors / bpb->bpbSecPerClust;

        if (clusterCount < 4085)
        {
            return FILE_SYSTEM_FAT_12;
        }
        else if (clusterCount < 65525)
        {
            return FILE_SYSTEM_FAT_16;
        }
        else
        {
            return FILE_SYSTEM_FAT_32;
        }
    }

    return FILE_SYSTEM_FAT_NONE;
}

struct direntry* GetFileCluster(const uint8_t* dataPtr, struct direntry* dirEntry, bpb33* bpb, uint32_t dirEntries, const char* filenameRemaining)
{
    char filename[12];

    size_t indexOfSeparator = IndexOfSeparator(filenameRemaining);
    size_t length = indexOfSeparator == (size_t)-1 ? strlen(filenameRemaining) : indexOfSeparator;

    if (length > 11)
    {
        _ASSERT(false);
        return NULL;
    }

    size_t indexOfDot = indexOfSeparator == (size_t)-1 ? IndexOfChar(filenameRemaining, '.') : (size_t)-1;

    filename[11] = '\0';
    size_t initialFillBytes = indexOfDot < length ? indexOfDot : length;
    memcpy(filename, filenameRemaining, initialFillBytes);

    size_t initialFill = indexOfDot == (size_t)-1 ? 11 : 8;
    for (size_t c = initialFillBytes; c < initialFill; c++)
    {
        filename[c] = ' ';
    }

    if (indexOfDot != (size_t)-1)
    {
        for (int c = 0; c < 3; c++)
        {
            filename[c + 8] = filenameRemaining[indexOfDot + c + 1];
        }
    }

    // Search for the file in the root directory
    for (uint32_t i = 0; i < dirEntries; i++)
    {
        direntry* de = &dirEntry[i];

        // LFN entries have 0xF flag
        if (de->deAttributes == 0xF)
        {
            winentry* we = (winentry*)de;
            //if( we->weCnt

            continue;
        }

        if (de->deAttributes == SLOT_EMPTY
            || de->deAttributes == SLOT_E5
            || de->deAttributes == SLOT_DELETED
            || de->deAttributes & ATTR_VOLUME)
        {
            continue;
        }

        if (de->deAttributes & ATTR_DIRECTORY)
        {
            //We're looking for a file if we've got no separators left
            if (indexOfSeparator == (size_t)-1)
            {
                continue;
            }
        }
        else
        {
            //We're looking for a directory if we've got at least one separator left
            if (indexOfSeparator != (size_t)-1)
            {
                continue;
            }
        }

        if (_strnicmp(filename, (const char*)de->deName, 11) == 0)
        {
            uint32_t unpackedSector = *(uint16_t*)de->deStartCluster | ((*(uint16_t*)de->deHighClust) << 16);

            uint32_t root_dir_sector = bpb->bpbResSectors + (bpb->bpbFATs * bpb->bpbFATsecs);

            uint32_t data_sector = root_dir_sector +
                (bpb->bpbRootDirEnts * 32 + bpb->bpbBytesPerSec - 1) / bpb->bpbBytesPerSec +
                (unpackedSector - 2) * bpb->bpbSecPerClust;

            if (de->deAttributes & ATTR_DIRECTORY)
            {
                const uint8_t* newPointer = dataPtr + bpb->bpbBytesPerSec * data_sector;
                uint32_t nextDirEntries = (bpb->bpbSecPerClust * bpb->bpbBytesPerSec) / sizeof(direntry);
                return GetFileCluster(dataPtr, (struct direntry*)newPointer, bpb, nextDirEntries, filenameRemaining + indexOfSeparator + 1);
            }
            else
            {
                return de;
            }
        }
    }

    return NULL;
}

#define IS_EOF(cluster, fatMask) (((cluster | ~fatMask) & CLUST_EOFS) == CLUST_EOFS)

uint32_t GetNextCluster(FILE_SYSTEM_FAT fsMode, const uint8_t* dataPtr, const bpb33* bpb, uint32_t cluster)
{
    const uint8_t* fatBase = dataPtr + bpb->bpbResSectors * bpb->bpbBytesPerSec;

    switch (fsMode)
    {
    case FILE_SYSTEM_FAT_12:
    {
        uint16_t fatEntry;

        // Correctly calculate FAT12 entry
        uint32_t fatOffset = cluster + (cluster / 2);  // Equivalent to cluster * 1.5

        //Prevent misaligned read by memcpying value
        memcpy(&fatEntry, fatBase + fatOffset, sizeof(uint16_t));

        if (cluster & 1)
        {
            fatEntry = fatEntry >> 4;
        }
        else
        {
            fatEntry = fatEntry & FAT12_MASK;
        }

        return fatEntry;
    }

    case FILE_SYSTEM_FAT_16:
        return FAT16_MASK & ((uint16_t*)fatBase)[cluster];

    case FILE_SYSTEM_FAT_32:
        return FAT32_MASK & ((uint32_t*)fatBase)[cluster];

    default:
        _ASSERT(false);
    }

    return 0;
}

void CopyFileToBuffer(FILE_SYSTEM_FAT fsMode, const uint8_t* dataPtr, const bpb33* bpb, uint32_t firstCluster, size_t fileSize, uint8_t* buffer, size_t bufferSize)
{
    uint32_t cluster = firstCluster;

    size_t offset = 0;

    uint32_t root_dir_sector = bpb->bpbResSectors + (bpb->bpbFATs * bpb->bpbFATsecs);

    uint32_t fatMask = 0;
    switch (fsMode)
    {
    case FILE_SYSTEM_FAT_12:
        fatMask = FAT12_MASK;
        break;

    case FILE_SYSTEM_FAT_16:
        fatMask = FAT16_MASK;
        break;

    case FILE_SYSTEM_FAT_32:
        fatMask = FAT32_MASK;
        break;

    default:
        _ASSERT(false);
    }

    while (!IS_EOF(cluster, fatMask))
    {
        _ASSERT(fileSize > 0);

        uint32_t root_dir_sector = bpb->bpbResSectors + (bpb->bpbFATs * bpb->bpbFATsecs);

        uint32_t data_sector = root_dir_sector +
            (bpb->bpbRootDirEnts * 32 + bpb->bpbBytesPerSec - 1) / bpb->bpbBytesPerSec +
            (cluster - 2) * bpb->bpbSecPerClust;

        const uint8_t* newPointer = dataPtr + bpb->bpbBytesPerSec * data_sector;

        size_t maxBytesToRead = bpb->bpbBytesPerSec * bpb->bpbSecPerClust;
        if (fileSize < maxBytesToRead)
        {
            maxBytesToRead = fileSize;
        }

        memcpy(buffer + offset, newPointer, maxBytesToRead);

        offset += maxBytesToRead;
        fileSize -= maxBytesToRead;

        cluster = GetNextCluster(fsMode, dataPtr, bpb, cluster);
    }
}
