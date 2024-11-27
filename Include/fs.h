#ifndef FS_H
#define FS_H
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
#define SECTOR_SIZE 512
#include "std.h"
typedef struct {
    uint8_t  jump_code[3];
    char     oem_name[8];
    uint16_t bytes_per_sector;
    uint8_t  sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t  num_fats;
    uint16_t root_entries;
    uint16_t short_sectors_count; // if zero check total_sectors_long
    uint8_t  media_descriptor;
    uint16_t fat_size_sectors;
    uint16_t sectors_per_track;
    uint16_t num_heads;
    uint32_t hidden_sectors;
    uint32_t long_sectors_count;
    uint8_t  drive_number;
    uint8_t  reserved1;
    uint8_t  boot_signature;
    uint32_t volume_id;
    char     volume_label[11];
    char     fs_type[8];
} __attribute__((packed)) BootSector;

typedef struct {
    char     filename[8];  
    char     ext[3];       // Extension
    uint8_t  attributes;   
    uint8_t  reserved;     // Reserved for system use
    uint8_t  create_time_tenths;
    uint16_t create_time;
    uint16_t create_date;
    uint16_t last_access_date;
    uint16_t cluster_index;  // Single 16-bit cluster index for FAT16
    uint16_t write_time;
    uint16_t write_date;
    uint32_t file_size;          // File size in bytes
} __attribute__((packed)) DirEntry;

#define ENTRY_AVAILABLE 0x00
#define ENTRY_ERASED 0xe5


uint32_t GetTotalSectorCount(uint8_t* image);
uint32_t GetMetaDataSector(uint8_t* image);
uint32_t GetClusterCount(uint8_t* image);
uint32_t GetImageSize(uint8_t* image);

uint16_t* GetTable(uint8_t* image,uint32_t fatIndex);
uint16_t GetClusterValue(uint8_t* image,uint32_t fatIndex, uint32_t clusterIndex);
void SetClusterValue(uint8_t* image,uint32_t fatIndex, uint32_t clusterIndex, uint16_t value);
uint32_t GetClusterOffset(uint8_t* image,uint32_t clusterIndex);
DirEntry* GetRootDirectory(uint8_t* image);


uint8_t *FatAllocImage(uint32_t iSize);
bool FatInitImage(uint8_t* image,uint8_t* bs);


void FatSplitPath(uint8_t dstName[8], uint8_t dstExt[3], const char *path);
uint16_t FatFindFreeCluster(uint8_t* image);
void FatUpdateCluster(uint8_t* image, uint32_t clusterIndex, uint16_t value);
DirEntry* FatFindFreeRootEntry(uint8_t* image);
void FatUpdateDirEntry(DirEntry *entry, uint16_t clusterIndex, const uint8_t name[8], const uint8_t ext[3], uint32_t fileSize);
void FatRemoveDirEntry(DirEntry *entry);
uint16_t FatAddData(uint8_t* image, void *data);
void FatRemoveData(uint8_t* image, uint32_t rootClusterIndex);
DirEntry* FatAddFile(uint8_t* image, const char *path, const void *data, uint32_t size);
void FatRemoveFile(uint8_t* image, DirEntry *entry);
uint16_t GetClusterIndex(DirEntry *entry);

#endif FS_H