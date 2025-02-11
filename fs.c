#include "fs.h"

HBA_PORT *port_ptr;
uint32_t GetTotalSectorCount()
{
    BootSector bpb;
    uint16_t tmp[256];
    read_ahci(port_ptr, 0, 0, 1, tmp);
    bpb = *(BootSector *)tmp;

    if (bpb.short_sectors_count)
    {
        return bpb.short_sectors_count;
    }
    else
    {
        return bpb.long_sectors_count;
    }
}

uint32_t
GetMetaDataSector()
{
    BootSector bpb;
    uint16_t tmp[256];
    read_ahci(port_ptr, 0, 0, 1, tmp);
    bpb = *(BootSector *)tmp;

    return (bpb.reserved_sectors +
            bpb.num_fats * bpb.fat_size_sectors +
            (bpb.root_entries * sizeof(DirEntry)) / bpb.bytes_per_sector);
}
uint32_t GetClusterCount()
{
    BootSector bpb;
    uint16_t tmp[256];
    read_ahci(port_ptr, 0, 0, 1, tmp);
    bpb = *(BootSector *)tmp;

    uint32_t totalSectorCount = GetTotalSectorCount();
    uint32_t metaSectorCount = GetMetaDataSector();
    uint32_t dataSectorCount = totalSectorCount - metaSectorCount;

    return dataSectorCount / bpb.sectors_per_cluster;
}

uint32_t GetImageSize()
{
    BootSector bpb;
    uint16_t tmp[256];
    read_ahci(port_ptr, 0, 0, 1, tmp);
    bpb = *(BootSector *)tmp;

    return GetTotalSectorCount() * bpb.bytes_per_sector;
}

uint32_t GetTable(uint32_t fatIndex)
{
    BootSector bpb;
    uint16_t tmp[256];
    read_ahci(port_ptr, 0, 0, 1, tmp);
    bpb = *(BootSector *)tmp;

    uint32_t offset = (bpb.reserved_sectors + fatIndex * bpb.fat_size_sectors);

    return (offset);
}

uint16_t GetClusterValue(uint32_t fatIndex, uint32_t clusterIndex)
{
    uint32_t fat = GetTable(fatIndex);
    uint16_t tmp[256];
    uint64_t sector = fat + clusterIndex / 256;
    read_ahci(port_ptr, (uint32_t)sector, sector >> 32, 1, tmp);

    return tmp[clusterIndex % 256];
}

void SetClusterValue(uint32_t fatIndex, uint32_t clusterIndex, uint16_t value)
{
    uint32_t fat = GetTable(fatIndex);
    uint16_t tmp[256];
    uint64_t sector = fat + clusterIndex / 256;
    read_ahci(port_ptr, (uint32_t)sector, sector >> 32, 1, tmp);

    tmp[clusterIndex % 256] = value;
    write_ahci(port_ptr, (uint32_t)sector, sector >> 32, 1, tmp);
}

uint32_t GetClusterOffset(uint32_t clusterIndex)
{
    BootSector bpb;
    uint16_t tmp[256];
    read_ahci(port_ptr, 0, 0, 1, tmp);
    bpb = *(BootSector *)tmp;
    uint32_t root_dir_sectors = (bpb.root_entries * 32) / bpb.bytes_per_sector;
    uint32_t data_start = (bpb.reserved_sectors + (bpb.num_fats * bpb.fat_size_sectors) + root_dir_sectors);
    return data_start + (clusterIndex - 2) * (bpb.sectors_per_cluster);
}

uint32_t GetRootDirectory()
{
    // check again
    BootSector bpb;
    uint16_t tmp[256];
    read_ahci(port_ptr, 0, 0, 1, tmp);
    bpb = *(BootSector *)tmp;

    uint32_t offset = (bpb.reserved_sectors + bpb.num_fats * bpb.fat_size_sectors);

    return (offset);
}

abool FatInitImage(HBA_PORT *port)
{
    // check
    port_ptr = port;

    BootSector bpb;

    uint16_t tmp[256];

    read_ahci(port_ptr, 0, 0, 1, tmp);
    bpb = *(BootSector *)tmp;

    // Initialize clusters
    uint32_t clusterCount = GetClusterCount();

    FatUpdateCluster(0, 0xff00 | bpb.media_descriptor); // media type
    FatUpdateCluster(1, 0xffff);                        // end of chain cluster

    return TRUE;
}

void FatSplitPath(uint8_t dstName[8], uint8_t dstExt[3], const char *path)
{
    uint32_t i;
    for (i = 0; i < 8; ++i)
    {
        dstName[i] = ' ';
    }
    for (i = 0; i < 3; ++i)
    {
        dstExt[i] = ' ';
    }
    // find filename
    const char *name = path;
    const char *p;
    for (p = path; *p != '\0'; ++p)
    {
        if (*p == '/')
        {
            name = p + 1;
        }
    }

    // Find the extension with spliting the .
    const char *ext = 0;
    for (p = name; *p != '\0'; ++p)
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

    for (i = 0; i < nameLen; ++i)
    {
        dstName[i] = toupper(name[i]);
    }

    // copy name and pad it for 3 char
    if (ext)
    {

        uint32_t extLen = strlen(ext);
        uint32_t i;
        if (extLen > 3)
        {
            extLen = 3;
        }
        for (i = 0; i < extLen; ++i)
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
    for (i = 0; i < clusterCount / 1024; i++)
    {
        read_ahci(port_ptr, fat, 0, 4, tmp);
        int j;
        for (j = 0; j < 1024; j++)
        {
            if (tmp[j] == 0)
            {
                return (i * 1024) + j;
            }
        }
    }

    return 0;
}

void FatUpdateCluster(uint32_t clusterIndex, uint16_t value)
{
    BootSector bpb;
    uint16_t tmp[256];
    read_ahci(port_ptr, 0, 0, 1, tmp);
    bpb = *(BootSector *)tmp;
    uint32_t fatIndex;
    for (fatIndex = 0; fatIndex < bpb.num_fats; ++fatIndex)
    {
        SetClusterValue(fatIndex, clusterIndex, value);
    }
}

uint32_t FatFindFreeRootEntry()
{
    BootSector bpb;
    uint16_t tmp[256];
    read_ahci(port_ptr, 0, 0, 1, tmp);
    bpb = *(BootSector *)tmp;

    uint32_t start = GetRootDirectory();
    uint32_t end = start + bpb.root_entries;
    int i;
    DirEntry tmp1[16];
    for (i = 0; i < bpb.root_entries / 16; i++)
    {
        read_ahci(port_ptr, start, 0, 1, (uint16_t *)tmp1);
        int j;
        for (j = 0; j < 16; j++)
        {
            if (!strlen(tmp1[j].filename))
            {
                return i * 16 + j;
            }
        }
    }

    return -1;
}

void FatUpdateDirEntry(uint32_t index, uint16_t clusterIndex, const uint8_t name[8], const uint8_t ext[3], uint32_t fileSize)
{

    DirEntry tmp[16];

    uint32_t root = GetRootDirectory();
    read_ahci(port_ptr, root + index / 16, 0, 1, (uint16_t *)tmp);
    unsigned int idx = index % 16;
    tmp[idx].low_cluster_index = clusterIndex;

    memcpy(tmp[idx].filename, name, sizeof(name));
    memcpy(tmp[idx].ext, ext, sizeof(ext));
    tmp[idx].file_size = fileSize;

    write_ahci(port_ptr, root + index / 16, 0, 1, (uint16_t *)tmp);
}

void FatRemoveDirEntry(uint32_t index)
{
    DirEntry tmp[16];
    uint32_t root = GetRootDirectory();
    read_ahci(port_ptr, root + index / 16, 0, 1, (uint16_t *)tmp);
    int idx = index % 16;
    tmp[idx].filename[0] = 0;
    write_ahci(port_ptr, root + index / 16, 0, 1, (uint16_t *)tmp);
}

uint16_t FatAddData(void *data, uint32_t size)
{
    BootSector bpb;
    uint16_t tmp[256];
    read_ahci(port_ptr, 0, 0, 1, tmp);
    bpb = *(BootSector *)tmp;
    uint32_t bytesPerCluster = bpb.sectors_per_cluster * bpb.bytes_per_sector;

    // Skip empty files
    if (size == 0)
    {
        return 0;
    }

    uint16_t endOfChainValue = GetClusterValue(0, 1);

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
        uint8_t tmp[512];
        read_ahci(port_ptr, offset, 0, 1, (uint16_t *)tmp);

        memcpy(tmp, p, count);
        write_ahci(port_ptr, offset, 0, 1, (uint16_t *)tmp);
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
        uint16_t nextClusterIndex = GetClusterValue(0, rootClusterIndex);
        FatUpdateCluster(rootClusterIndex, 0);
        rootClusterIndex = nextClusterIndex;
    }
}

uint32_t FatAddFile(const char *path, const void *data, uint32_t size)
{
    // Find Directory Entry
    uint32_t entry = FatFindFreeRootEntry();
    if (entry == (uint32_t)-1)
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

void FatRemoveFile(uint32_t index)
{
    DirEntry tmp[16];
    uint32_t root = GetRootDirectory();
    read_ahci(port_ptr, root + index / 16, 0, 1, (uint16_t *)tmp);
    int idx = index % 16;
    FatRemoveData(tmp[idx].low_cluster_index);
    FatRemoveDirEntry(index);
}

int getContent(const char* path, void* arr)
{
    char name[8];
    char ext[3];
    DirEntry tmp[32];
    FatSplitPath(name,ext,path);
    uint32_t root = GetRootDirectory();
    read_ahci(port_ptr,root,0,2,(uint16_t*)tmp);

    for(int i = 0; i < 32; i++)
    {
        DirEntry x = tmp[i];
        
        if(!fstrcmp(x.filename,name))
        {
            uint16_t cluster_idx = x.low_cluster_index;
            int cluster_offset;
            int cluster_value;
            int arr_idx = 0;
            while (1)
            {
                cluster_offset = GetClusterOffset(cluster_idx);
                cluster_value = GetClusterValue(0,cluster_idx);
                read_ahci(port_ptr,cluster_offset,0,1,(uint16_t*)(arr + arr_idx));
                if(cluster_value == 0xffff)
                {
                    break;
                }
                arr_idx += 512;
                cluster_idx = cluster_value;
            }
            break;
        }
    }
}




