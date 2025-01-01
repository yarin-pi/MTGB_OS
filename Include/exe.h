#ifndef EXE_H
#define EXE_H

#include "std.h"

#define ELF_MAGIC 0x464C457F // ELF magic number (0x7F followed by "ELF")
#define ELF_CLASS_32 1       // 32-bit ELF
#define ELF_CLASS_64 2       // 64-bit ELF
#define ELF_DATA_LSB 1       // Little-endian
#define ELF_DATA_MSB 2       // Big-endian

// ELF Header structure
typedef struct elf_header
{
    uint32_t magic;      // Magic number to identify ELF
    uint8_t eclass;      // 1: 32-bit, 2: 64-bit
    uint8_t data;        // Endianness: 1=LSB, 2=MSB
    uint8_t version;     // ELF version
    uint8_t os_abi;      // OS/ABI identification
    uint8_t abi_version; // ABI version
    uint8_t pad[7];      // Padding bytes
    uint16_t type;       // Object file type (e.g., executable, relocatable)
    uint16_t machine;    // Architecture (e.g., x86, ARM)
    uint32_t version_2;  // Version again
    uint32_t entry;      // Entry point of the program
    uint32_t phoff;      // Program header table offset
    uint32_t shoff;      // Section header table offset
    uint32_t flags;      // Processor-specific flags
    uint16_t ehsize;     // ELF header size
    uint16_t phentsize;  // Program header entry size
    uint16_t phnum;      // Number of program header entries
    uint16_t shentsize;  // Section header entry size
    uint16_t shnum;      // Number of section header entries
    uint16_t shstrndx;   // Index of the section header string table
} ELFHeader;

// Program Header structure (used to describe memory layout)
typedef struct program_header
{
    uint32_t type;   // Type of segment (e.g., loadable, dynamic linking)
    uint32_t offset; // Offset of segment in file
    uint32_t vaddr;  // Virtual address in memory
    uint32_t paddr;  // Physical address (if applicable)
    uint32_t filesz; // Size of segment in file
    uint32_t memsz;  // Size of segment in memory
    uint32_t flags;  // Flags (e.g., executable, writable)
    uint32_t align;  // Alignment of segment
} ProgramHeader;

int validate_elf(ELFHeader *header);

#endif // EXE_H
