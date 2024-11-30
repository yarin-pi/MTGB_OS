#include "fs.h"

uint32_t GetTotalSectorCount()
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
uint32_t GetMetaDataSector()
{
    uint8_t tmp[512];
    read_ahci(0,0,0,1,(uint16_t*)tmp);
    uint8_t* ptr = tmp + 0xb;
    BootSector* bpb = (BootSector*)(ptr);

    return (bpb->reserved_sectors +
            bpb->num_fats * bpb->fat_size_sectors +
            (bpb->root_entries * sizeof(DirEntry)) / bpb->bytes_per_sector);
}
uint32_t GetClusterCount()
{
    uint8_t tmp[512];
    read_ahci(0,0,0,1,(uint16_t*)tmp);
    uint8_t* ptr = tmp + 0xb;
    BootSector* bpb = (BootSector*)(ptr);

    uint32_t totalSectorCount = GetTotalSectorCount();
    uint32_t metaSectorCount = GetMetaSectorCount();
    uint32_t dataSectorCount = totalSectorCount - metaSectorCount;

    return dataSectorCount / bpb->sectors_per_cluster;
}

uint32_t GetImageSize()
{
    uint8_t tmp[512];
    read_ahci(0,0,0,1,(uint16_t*)tmp);
    uint8_t* ptr = tmp + 0xb;
    BootSector* bpb = (BootSector*)(ptr);

    return GetTotalSectorCount() * bpb->bytes_per_sector;
}

uint32_t GetTable(uint32_t fatIndex)
{
    uint8_t tmp[512];
    read_ahci(0,0,0,1,(uint16_t*)tmp);
    uint8_t* ptr = tmp + 0xb;
    BootSector* bpb = (BootSector*)(ptr); 

    uint32_t offset = (bpb->reserved_sectors + fatIndex * bpb->fat_size_sectors);

    return (offset);
}

uint16_t GetClusterValue(uint32_t fatIndex, uint32_t clusterIndex)
{
    uint32_t fat = GetTable(fatIndex);
    uint16_t tmp[256];
    uint64_t sector = fat + clusterIndex/256;
    read_ahci(0,(uint32_t )sector,sector >> 32,1,tmp);

    return tmp[clusterIndex % 256];
}

void SetClusterValue(uint32_t fatIndex, uint32_t clusterIndex, uint16_t value)
{
    uint32_t fat = GetTable(fatIndex);
    uint16_t tmp[256];
    uint64_t sector = fat + clusterIndex/256;
    read_ahci(0,(uint32_t )sector,sector >> 32,1,tmp);
    
    tmp[clusterIndex % 256] = value;
    write_ahci(0,(uint32_t)sector,sector >> 32,1,tmp);

    
}

uint32_t GetClusterOffset( uint32_t clusterIndex)
{
    uint8_t tmp[512];
    read_ahci(0,0,0,1,(uint16_t*)tmp);
    uint8_t* ptr = tmp + 0xb;
    BootSector* bpb = (BootSector*)(ptr); 

    return (bpb->reserved_sectors + bpb->num_fats * bpb->fat_size_sectors) * bpb->bytes_per_sector +
           bpb->root_entries * sizeof(DirEntry) +
           (clusterIndex ) * (bpb->sectors_per_cluster * bpb->bytes_per_sector);
}

uint32_t GetRootDirectory()
{
    //check again
    uint8_t tmp[512];
    read_ahci(0,0,0,1,(uint16_t*)tmp);
    uint8_t* ptr = tmp + 0xb;
    BootSector* bpb = (BootSector*)(ptr); 

    uint32_t offset = (bpb->reserved_sectors + bpb->num_fats * bpb->fat_size_sectors);
    
    return (offset);
}



bool FatInitImage(uint8_t *bs)
{
    //check
    BootSector *bpb = (BootSector *)bs;

    // Validate signature
    if (bs[0x1fe] != 0x55 || bs[0x1ff] != 0xaa)
    {
        return false;
    }

    // Copy to sector 0
    memcpy(image, bs, bpb->bytes_per_sector);

    // Initialize clusters
    uint32_t clusterCount = GetClusterCount();

    FatUpdateCluster(0, 0xff00 | bpb->media_descriptor); // media type
    FatUpdateCluster(1, 0xffff);                         // end of chain cluster

    for (uint32_t clusterIndex = 2; clusterIndex < clusterCount; ++clusterIndex)
    {
        FatUpdateCluster(clusterIndex, 0x0000);
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
    const char *p ;
    for (p = path; *p != '\0'; ++p)
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

uint16_t FatFindFreeCluster()
{
    uint32_t clusterCount = GetClusterCount();

    uint32_t fat = GetTable(0);
    uint16_t tmp[1024];
    int i;
    for (i = 0; i < clusterCount/1024; i++)
    {
        read_ahci(0,fat,0,4,tmp);
        int j;
        for (j = 0; j < 1024; j++)
        {
            if (tmp[j] == 0)
            {
                return (i*1024) + j;
            }
        }
    }

    return 0;
}

void FatUpdateCluster(uint32_t clusterIndex, uint16_t value)
{
    uint8_t tmp[512];
    read_ahci(0,0,0,1,(uint16_t*)tmp);
    uint8_t* ptr = tmp + 0xb;
    BootSector* bpb = (BootSector*)(ptr); 
    uint32_t fatIndex;
    for (fatIndex = 0; fatIndex < bpb->num_fats; ++fatIndex)
    {
        SetClusterValue(fatIndex, clusterIndex, value);
    }
}

uint32_t FatFindFreeRootEntry()
{
    uint8_t tmp[512];
    read_ahci(0,0,0,1,(uint16_t*)tmp);
    uint8_t* ptr = tmp + 0xb;
    BootSector* bpb = (BootSector*)(ptr); 

    uint32_t start = GetRootDirectory();
    uint32_t end = start + bpb->root_entries;
    int i;
    DirEntry tmp[16];
    for (i = 0;i<bpb->root_entries / 16;i++)
    {
        read_ahci(0,start,0,1,(uint16_t*)tmp);
        int j;
        for(j = 0;j<16;j++)
        {
            if(!strlen(tmp[j]->filename))
            {
                return i*16 + j;
            }
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

uint16_t FatAddData(void *data, uint32_t size)
{
    BootSector *bpb = (BootSector *)image;
    uint32_t bytesPerCluster = bpb->sectors_per_cluster * bpb->bytes_per_sector;

    // Skip empty files
    if (size == 0)
    {
        return 0;
    }

    uint16_t endOfChainValue = FatGetClusterValue(0, 1);

    uint16_t prevClusterIndex = 0;
    uint16_t rootClusterIndex = 0;

    // Copy data one cluster at a time.
    const uint8_t *p = (const uint8_t *)data;
    const uint8_t *end = p + size;
    while (p < end)
    {
        // Find a free cluster
        uint16_t clusterIndex = FatFindFreeCluster();
        if (clusterIndex == 0)
        {
            // Ran out of disk space, free allocated clusters
            if (rootClusterIndex != 0)
            {
                FatRemoveData(rootClusterIndex);
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
        uint32_t offset = GetClusterOffset(clusterIndex);
        memcpy(image + offset, p, count);
        p += count;

        // Update FAT clusters
        FatUpdateCluster(clusterIndex, endOfChainValue);
        if (prevClusterIndex)
        {
            FatUpdateCluster(prevClusterIndex, clusterIndex);
        }
        else
        {
            rootClusterIndex = clusterIndex;
        }

        prevClusterIndex = clusterIndex;
    }

    return rootClusterIndex;
}

void FatRemoveData(uint32_t rootClusterIndex)
{
    uint16_t endOfChainValue = GetClusterValue(0, 1);

    while (rootClusterIndex != endOfChainValue)
    {
        uint16_t nextClusterIndex = GetClusterValue( 0, rootClusterIndex);
        FatUpdateCluster(rootClusterIndex, 0);
        rootClusterIndex = nextClusterIndex;
    }
}

DirEntry *FatAddFile(const char *path, const void *data, uint32_t size)
{
    // Find Directory Entry
    DirEntry *entry = FatFindFreeRootEntry();
    if (!entry)
    {
        return 0;
    }

    // Add File
    uint16_t rootClusterIndex = FatAddData(data, size);
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

void FatRemoveFile(DirEntry *entry)
{
    FatRemoveData(entry->cluster_index);
    FatRemoveDirEntry(entry);
}
