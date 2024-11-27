#include "fs.h"


uint32_t GetTotalSectorCount(uint8_t* image)
{
    BootSector *bpb = (BootSector *)image;

    if (bpb->short_sectors_count)
    {
        return bpb->short_sectors_count;
    }
    else
    {
        return bpb->long_sectors_count;
    }
}
uint32_t GetMetaDataSector(uint8_t* image)
{
    BootSector *bpb = (BootSector *)image;

    return
        (bpb->reserved_sectors +
        bpb->num_fats * bpb->fat_size_sectors +
        (bpb->root_entries * sizeof(DirEntry)) / bpb->bytes_per_sector);
}
uint32_t GetClusterCount(uint8_t* image)
{
    BootSector *bpb = (BootSector *)image;

    uint32_t totalSectorCount = GetTotalSectorCount(image);
    uint32_t metaSectorCount = GetMetaSectorCount(image);
    uint32_t dataSectorCount = totalSectorCount - metaSectorCount;

    return dataSectorCount / bpb->sectors_per_cluster;
}

uint32_t GetImageSize(uint8_t *image)
{
    BootSector *bpb = (BootSector *)image;

    return GetTotalSectorCount(image) * bpb->bytes_per_sector;
}

uint16_t* GetTable(uint8_t* image,uint32_t fatIndex);
{
    bootSector *bpb = (bootSector *)image;

    uint offset = (bpb->reserved_sectors + fatIndex * bpb->fat_size_sectors) * bpb->bytes_per_sector;

    return (uint16_t *)(image + offset);
}

uint16_t GetClusterValue(uint8_t* image,uint32_t fatIndex, uint32_t clusterIndex)
{
    uint16_t *fat = GetTable(image, fatIndex);

    return fat[clusterIndex];
}

void SetClusterValue(uint8_t* image,uint32_t fatIndex, uint32_t clusterIndex, uint16_t value)
{
    uint16_t *fat = GetTable(image, fatIndex);

    assert(clusterIndex < FatGetClusterCount(image));

    fat[clusterIndex] = value;
}

uint32_t GetClusterOffset(uint8_t* image,uint32_t clusterIndex)
{
    BootSector *bpb = (BootSector *)image;

    return
        (bpb->reserved_sectors + bpb->num_fats * bpb->fat_size_sectors) * bpb->bytes_per_sector +
        bpb->root_entries * sizeof(DirEntry) +
        (clusterIndex - 2) * (bpb->sectors_per_cluster * bpb->bytes_per_sector);
}

DirEntry* GetRootDirectory(uint8_t* image)
{
    BiosParamBlock *bpb = (BiosParamBlock *)image;

    uint offset = (bpb->reservedSectorCount + bpb->fatCount * bpb->fat_size_sectors) * bpb->bytes_per_sector;
    uint dataSize = bpb->root_entries * sizeof(DirEntry);
    return (DirEntry *)(image + offset);
}

uint8_t* FatAllocImage(uint32_t iSize)
{
    uint8_t *image = ;
    memset(image, ENTRY_ERASED, imageSize);
    return image;
}

bool FatInitImage(uint8_t* image,uint8_t* BootSector)
{

}