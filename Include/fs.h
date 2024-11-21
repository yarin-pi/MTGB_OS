#ifndef FS_H
#define FS_H
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
#define SECTOR_SIZE 512

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
} BootSector;

typedef struct {
    char     filename[8];  
    char     ext[3];       // Extension
    uint8_t  attributes;   
    uint8_t  reserved;     // Reserved for system use
    uint8_t  create_time_tenths;
    uint16_t create_time;
    uint16_t create_date;
    uint16_t last_access_date;
    uint16_t first_cluster_high; // always 0 for FAT16
    uint16_t write_time;
    uint16_t write_date;
    uint16_t first_cluster_low;  
    uint32_t file_size;          // File size in bytes
} DirEntry;

#endif FS_H