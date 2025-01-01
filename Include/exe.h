#ifndef EXE_H
#define EXE_H

#include "std.h"
#define ELF_NIDENT 16
enum Elf_Ident
{
    EI_MAG0 = 0,       // 0x7F
    EI_MAG1 = 1,       // 'E'
    EI_MAG2 = 2,       // 'L'
    EI_MAG3 = 3,       // 'F'
    EI_CLASS = 4,      // Architecture (32/64)
    EI_DATA = 5,       // Byte Order
    EI_VERSION = 6,    // ELF Version
    EI_OSABI = 7,      // OS Specific
    EI_ABIVERSION = 8, // OS Specific
    EI_PAD = 9         // Padding
};
enum Elf_Type
{
    ET_NONE = 0, // Unkown Type
    ET_REL = 1,  // Relocatable File
    ET_EXEC = 2  // Executable File
};

#define EM_386 (3)     // x86 Machine Type
#define EV_CURRENT (1) // ELF Current Version

#define ELFMAG0 0x7F // e_ident[EI_MAG0]
#define ELFMAG1 'E'  // e_ident[EI_MAG1]
#define ELFMAG2 'L'  // e_ident[EI_MAG2]
#define ELFMAG3 'F'  // e_ident[EI_MAG3]

#define ELFDATA2LSB (1) // Little Endian
#define ELFCLASS32 (1)  // 32-bit Architecture

// ELF Header structure
typedef struct elf_header
{
    uint8_t e_ident[ELF_NIDENT]; // Identification bytes
    uint16_t e_type;             // File type
    uint16_t e_machine;          // Machine type
    uint32_t e_version;          // ELF version
    uint32_t e_entry;            // Entry point address
    uint32_t e_phoff;            // Program header table offset
    uint32_t e_shoff;            // Section header table offset
    uint32_t e_flags;            // Flags
    uint16_t e_ehsize;           // ELF header size
    uint16_t e_phentsize;        // Size of one program header entry
    uint16_t e_phnum;            // Number of program header entries
    uint16_t e_shentsize;        // Size of one section header entry
    uint16_t e_shnum;            // Number of section header entries
    uint16_t e_shstrndx;         // Section header string table index
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
