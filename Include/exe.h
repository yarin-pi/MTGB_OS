#ifndef EXE_H
#define EXE_H
#include "std.h"
#include "print.h"
#include "vm.h"
#include "gdt.h"
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
#define ELF_NIDENT 16
#define ELF_RELOC_ERR -1
#define ELF32_R_SYM(INFO) ((INFO) >> 8)
#define ELF32_R_TYPE(INFO) ((uint8_t)(INFO))
#define DO_386_32(S, A) ((S) + (A))
#define DO_386_PC32(S, A, P) ((S) + (A) - (P))
extern uint32_t k_stack;
enum RtT_Types
{
    R_386_NONE = 0, // No relocation
    R_386_32 = 1,   // Symbol + Offset
    R_386_PC32 = 2  // Symbol + Offset - Section Offset
};

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
    ET_NONE = 0, // Unknown Type
    ET_REL = 1,  // Relocatable File
    ET_EXEC = 2  // Executable File
};

typedef struct
{
    uint32_t sh_name;
    uint32_t sh_type;
    uint32_t sh_flags;
    uint32_t sh_addr;
    uint32_t sh_offset;
    uint32_t sh_size;
    uint32_t sh_link;
    uint32_t sh_info;
    uint32_t sh_addralign;
    uint32_t sh_entsize;
} SectionHeader;

#define SHN_UNDEF (0x00) // Undefined/Not Present
#define SHN_ABS 0xFFF1   // Absolute symbol

typedef struct
{
    uint32_t r_offset;
    uint32_t r_info;
} Elf32_Rel;

typedef struct
{
    uint32_t r_offset;
    uint32_t r_info;
    uint32_t r_addend;
} Elf32_Rela;

enum ShT_Types
{
    SHT_0 = 0,        // 0 section
    SHT_PROGBITS = 1, // Program information
    SHT_SYMTAB = 2,   // Symbol table
    SHT_STRTAB = 3,   // String table
    SHT_RELA = 4,     // Relocation (w/ addend)
    SHT_NOBITS = 8,   // Not present in file
    SHT_REL = 9,      // Relocation (no addend)
};

enum ShT_Attributes
{
    SHF_WRITE = 0x01, // Writable section
    SHF_ALLOC = 0x02  // Exists in memory
};

typedef struct
{
    uint32_t st_name;
    uint32_t st_value;
    uint32_t st_size;
    uint8_t st_info;
    uint8_t st_other;
    uint16_t st_shndx;
} SymbolHeader;

#define ELF32_ST_BIND(INFO) ((INFO) >> 4)
#define ELF32_ST_TYPE(INFO) ((INFO) & 0x0F)

enum StT_Bindings
{
    STB_LOCAL = 0,  // Local scope
    STB_GLOBAL = 1, // Global scope
    STB_WEAK = 2    // Weak, (ie. __attribute__((weak)))
};

enum StT_Types
{
    STT_NOTYPE = 0, // No type
    STT_OBJECT = 1, // Variables, arrays, etc.
    STT_FUNC = 2    // Methods or functions
};

#define EM_386 (3)     // x86 Machine Type
#define EV_CURRENT (1) // ELF Current Version

#define ELFMAG0 0x7F // e_ident[EI_MAG0]
#define ELFMAG1 'E'  // e_ident[EI_MAG1]
#define ELFMAG2 'L'  // e_ident[EI_MAG2]
#define ELFMAG3 'F'  // e_ident[EI_MAG3]

#define ELFDATA2LSB (1) // Little Endian
#define ELFCLASS32 (1)  // 32-bit Architecture
#define PT_LOAD 1

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

// Function declarations
uint32_t validate_elf(ELFHeader *header);
SectionHeader *elf_sheader(ELFHeader *hdr);
SectionHeader *elf_section(ELFHeader *hdr, int idx);
uint32_t elf_load_stage1(ELFHeader *hdr);
uint32_t elf_load_stage2(ELFHeader *hdr);

page_directory_entry_t *parse_program_headers(ELFHeader *hdr);
struct kthread *elf_load_file(void *file);
void *elf_load_rel(ELFHeader *hdr);
uint32_t elf_do_reloc(ELFHeader *hdr, Elf32_Rel *rel, SectionHeader *reltab);
void *elf_lookup_symbol(const char *name, ELFHeader *hdr);
uint32_t elf_get_symval(ELFHeader *hdr, int table, uint32_t idx);

#endif // EXE_H
