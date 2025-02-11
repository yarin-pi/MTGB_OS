#ifndef FS_H
#define FS_H

#define SECTOR_SIZE 512
#include "std.h"
#include "ahci.h"
typedef struct {
    char jmp[3];
    char blabla[8];
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
    char     fs_type[9];
} __attribute__((packed)) BootSector; //add __attribute__((packed))

typedef struct {
    char     filename[8];  
    char     ext[3];       // Extension
    uint8_t  attributes;   
    uint8_t  reserved;     // Reserved for system use
    uint8_t  create_time_tenths;
    uint16_t create_time;
    uint16_t create_date;
    uint16_t last_access_date;
    uint16_t high_cluster_index;  // Single 16-bit cluster index for FAT16
    uint16_t write_time;
    uint16_t write_date;
    uint16_t low_cluster_index;
    uint32_t file_size;        
} __attribute__((packed)) DirEntry; //add __attribute__((packed))




uint32_t GetTotalSectorCount();
uint32_t GetMetaDataSector();
uint32_t GetClusterCount();
uint32_t GetImageSize();

uint32_t GetTable(uint32_t fatIndex);
uint16_t GetClusterValue(uint32_t fatIndex, uint32_t clusterIndex);
void SetClusterValue(uint32_t fatIndex, uint32_t clusterIndex, uint16_t value);
uint32_t GetClusterOffset(uint32_t clusterIndex);
uint32_t GetRootDirectory();



abool FatInitImage(HBA_PORT* port);


void FatSplitPath(uint8_t dstName[8], uint8_t dstExt[3], const char *path);
uint16_t FatFindFreeCluster();
void FatUpdateCluster(uint32_t clusterIndex, uint16_t value);
uint32_t FatFindFreeRootEntry();
void FatUpdateDirEntry(uint32_t entry, uint16_t clusterIndex, const uint8_t name[8], const uint8_t ext[3], uint32_t fileSize);
void FatRemoveDirEntry(uint32_t entry);
uint16_t FatAddData( void *data, uint32_t size);
void FatRemoveData( uint32_t rootClusterIndex);
uint32_t FatAddFile(const char *path, const void *data, uint32_t size);
void FatRemoveFile(uint32_t entry);
int getContent(const char* path, void* arr);
#endif FS_H