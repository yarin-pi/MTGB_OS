#include "fs.h"

uint32_t GetTotalSectorCount(uint8_t *image)
{
    uint8_t tmp[512];
    read_ahci(0,0,0,1,(uint16_t*)tmp);
    uint8_t* ptr = tmp + 0xb;
    BootSector* bpb = (BootSector*)(ptr);
    


    if (bpb->short_sectors_count)
    {
        return bpb->short_sectors_count;
    }
    else
    {
        return bpb->long_sectors_count;
    }
}
uint32_t GetMetaDataSector(uint8_t *image)
{
    uint8_t tmp[512];
    read_ahci(0,0,0,1,(uint16_t*)tmp);
    uint8_t* ptr = tmp + 0xb;
    BootSector* bpb = (BootSector*)(ptr);

    return (bpb->reserved_sectors +
            bpb->num_fats * bpb->fat_size_sectors +
            (bpb->root_entries * sizeof(DirEntry)) / bpb->bytes_per_sector);
}
uint32_t GetClusterCount(uint8_t *image)
{
    uint8_t tmp[512];
    read_ahci(0,0,0,1,(uint16_t*)tmp);
    uint8_t* ptr = tmp + 0xb;
    BootSector* bpb = (BootSector*)(ptr);

    uint32_t totalSectorCount = GetTotalSectorCount(image);
    uint32_t metaSectorCount = GetMetaSectorCount(image);
    uint32_t dataSectorCount = totalSectorCount - metaSectorCount;

    return dataSectorCount / bpb->sectors_per_cluster;
}

uint32_t GetImageSize(uint8_t *image)
{
    uint8_t tmp[512];
    read_ahci(0,0,0,1,(uint16_t*)tmp);
    uint8_t* ptr = tmp + 0xb;
    BootSector* bpb = (BootSector*)(ptr);

    return GetTotalSectorCount(image) * bpb->bytes_per_sector;
}

uint16_t *GetTable(uint8_t *image, uint32_t fatIndex)
{
    BootSector *bpb = (BootSector *)image;

    uint32_t offset = (bpb->reserved_sectors + fatIndex * bpb->fat_size_sectors) * bpb->bytes_per_sector;

    return (uint16_t *)(image + offset);
}

uint16_t GetClusterValue(uint8_t *image, uint32_t fatIndex, uint32_t clusterIndex)
{
    uint16_t *fat = GetTable(image, fatIndex);

    return fat[clusterIndex];
}

void SetClusterValue(uint8_t *image, uint32_t fatIndex, uint32_t clusterIndex, uint16_t value)
{
    uint16_t *fat = GetTable(image, fatIndex);

    assert(clusterIndex < FatGetClusterCount(image));

    fat[clusterIndex] = value;
}

uint32_t GetClusterOffset(uint8_t *image, uint32_t clusterIndex)
{
    BootSector *bpb = (BootSector *)image;

    return (bpb->reserved_sectors + bpb->num_fats * bpb->fat_size_sectors) * bpb->bytes_per_sector +
           bpb->root_entries * sizeof(DirEntry) +
           (clusterIndex - 2) * (bpb->sectors_per_cluster * bpb->bytes_per_sector);
}

DirEntry *GetRootDirectory(uint8_t *image)
{
    BootSector *bpb = (BootSector *)image;

    uint32_t offset = (bpb->reserved_sectors + bpb->num_fats * bpb->fat_size_sectors) * bpb->bytes_per_sector;
    uint32_t dataSize = bpb->root_entries * sizeof(DirEntry);
    return (DirEntry *)(image + offset);
}

uint8_t *FatAllocImage(uint32_t iSize)
{
    uint8_t *image = malloc; // need to be updated
    memset(image, ENTRY_ERASED, iSize);
    return image;
}

bool FatInitImage(uint8_t *image, uint8_t *bs)
{
    BootSector *bpb = (BootSector *)bs;

    // Validate signature
    if (bs[0x1fe] != 0x55 || bs[0x1ff] != 0xaa)
    {
        return false;
    }

    // Copy to sector 0
    memcpy(image, bs, bpb->bytes_per_sector);

    // Initialize clusters
    uint32_t clusterCount = GetClusterCount(image);

    FatUpdateCluster(image, 0, 0xff00 | bpb->media_descriptor); // media type
    FatUpdateCluster(image, 1, 0xffff);                         // end of chain cluster

    for (uint32_t clusterIndex = 2; clusterIndex < clusterCount; ++clusterIndex)
    {
        FatUpdateCluster(image, clusterIndex, 0x0000);
    }

    return true;
}

void FatSplitPath(uint8_t dstName[8], uint8_t dstExt[3], const char *path)
{
    for (uint32_t i = 0; i < 8; ++i)
    {
        dstName[i] = ' ';
    }
    for (uint32_t i = 0; i < 3; ++i)
    {
        dstExt[i] = ' ';
    }
    // find filename
    const char *name = path;
    for (const char *p = path; *p != '\0'; ++p)
    {
        if (*p == '/')
        {
            name = p + 1;
        }
    }

    // Find the extension with spliting the .
    const char *ext = 0;
    for (const char *p = name; *p != '\0'; ++p)
    {
        if (*p == '.')
        {
            ext = p + 1;
            break;
        }
    }

    // copy name and pad it for 8 char
    uint32_t nameLen;
    if (ext)
    {
        nameLen = (uint32_t)(ext - name - 1);
    }
    else
    {
        nameLen = strlen(name);
    }

    if (nameLen > 8)
    {
        nameLen = 8;
    }

    for (uint32_t i = 0; i < nameLen; ++i)
    {
        dstName[i] = toupper(name[i]);
    }

    // copy name and pad it for 3 char
    if (ext)
    {
        uint32_t extLen = strlen(ext);
        if (extLen > 3)
        {
            extLen = 3;
        }
        for (uint32_t i = 0; i < extLen; ++i)
        {
            dstExt[i] = toupper(ext[i]);
        }
    }
}

uint16_t FatFindFreeCluster(uint8_t *image)
{
    uint32_t clusterCount = GetClusterCount(image);

    uint16_t *fat = GetTable(image, 0);

    for (uint32_t clusterIndex = 2; clusterIndex < clusterCount; ++clusterIndex)
    {
        uint16_t data = fat[clusterIndex];
        if (data == 0)
        {
            return clusterIndex;
        }
    }

    return 0;
}

void FatUpdateCluster(uint8_t *image, uint32_t clusterIndex, uint16_t value)
{
    BootSector *bpb = (BootSector *)image;

    for (uint32_t fatIndex = 0; fatIndex < bpb->num_fats; ++fatIndex)
    {
        SetClusterValue(image, fatIndex, clusterIndex, value);
    }
}

DirEntry *FatFindFreeRootEntry(uint8_t *image)
{
    BootSector *bpb = (BootSector *)image;

    DirEntry *start = GetRootDirectory(image);
    DirEntry *end = start + bpb->root_entries;

    for (DirEntry *entry = start; entry != end; ++entry)
    {
        uint8_t mark = entry->filename[0];
        if (mark == ENTRY_AVAILABLE || mark == ENTRY_ERASED)
        {
            return entry;
        }
    }

    return 0;
}

void FatUpdateDirEntry(DirEntry *entry, uint16_t clusterIndex, const uint8_t name[8], const uint8_t ext[3], uint32_t fileSize)
{
    entry->cluster_index = clusterIndex;
    memcpy(entry->filename, name, sizeof(entry->filename));
    memcpy(entry->ext, ext, sizeof(entry->ext));
    entry->file_size = fileSize;
}

void FatRemoveDirEntry(DirEntry *entry)
{
    entry->filename[0] = ENTRY_AVAILABLE;
}

uint16_t FatAddData(uint8_t *image, void *data, uint32_t size)
{
    BootSector *bpb = (BootSector *)image;
    uint32_t bytesPerCluster = bpb->sectors_per_cluster * bpb->bytes_per_sector;

    // Skip empty files
    if (size == 0)
    {
        return 0;
    }

    uint16_t endOfChainValue = FatGetClusterValue(image, 0, 1);

    uint16_t prevClusterIndex = 0;
    uint16_t rootClusterIndex = 0;

    // Copy data one cluster at a time.
    const uint8_t *p = (const uint8_t *)data;
    const uint8_t *end = p + size;
    while (p < end)
    {
        // Find a free cluster
        uint16_t clusterIndex = FatFindFreeCluster(image);
        if (clusterIndex == 0)
        {
            // Ran out of disk space, free allocated clusters
            if (rootClusterIndex != 0)
            {
                FatRemoveData(image, rootClusterIndex);
            }

            return 0;
        }

        // Determine amount of data to copy
        uint32_t count = end - p;
        if (count > bytesPerCluster)
        {
            count = bytesPerCluster;
        }

        // Transfer bytes into image at cluster location
        uint32_t offset = GetClusterOffset(image, clusterIndex);
        memcpy(image + offset, p, count);
        p += count;

        // Update FAT clusters
        FatUpdateCluster(image, clusterIndex, endOfChainValue);
        if (prevClusterIndex)
        {
            FatUpdateCluster(image, prevClusterIndex, clusterIndex);
        }
        else
        {
            rootClusterIndex = clusterIndex;
        }

        prevClusterIndex = clusterIndex;
    }

    return rootClusterIndex;
}

void FatRemoveData(uint8_t *image, uint32_t rootClusterIndex)
{
    uint16_t endOfChainValue = GetClusterValue(image, 0, 1);

    while (rootClusterIndex != endOfChainValue)
    {
        uint16_t nextClusterIndex = GetClusterValue(image, 0, rootClusterIndex);
        FatUpdateCluster(image, rootClusterIndex, 0);
        rootClusterIndex = nextClusterIndex;
    }
}

DirEntry *FatAddFile(uint8_t *image, const char *path, const void *data, uint32_t size)
{
    // Find Directory Entry
    DirEntry *entry = FatFindFreeRootEntry(image);
    if (!entry)
    {
        return 0;
    }

    // Add File
    uint16_t rootClusterIndex = FatAddData(image, data, size);
    if (!rootClusterIndex)
    {
        return 0;
    }

    // Update Directory Entry
    uint8_t name[8];
    uint8_t ext[3];
    FatSplitPath(name, ext, path);

    FatUpdateDirEntry(entry, rootClusterIndex, name, ext, size);
    return entry;
}

void FatRemoveFile(uint8_t *image, DirEntry *entry)
{
    FatRemoveData(image, entry->cluster_index);
    FatRemoveDirEntry(entry);
}
