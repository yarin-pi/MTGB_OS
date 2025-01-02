#include "exe.h"
#include "std.h"
#include "print.h"
#include "process.h"
#include "pm.h"
int validate_elf(ELFHeader *header)
{
    if (!header)
        return FALSE;

    // Check magic number
    if (header->e_ident[EI_MAG0] != ELFMAG0 ||
        header->e_ident[EI_MAG1] != ELFMAG1 ||
        header->e_ident[EI_MAG2] != ELFMAG2 ||
        header->e_ident[EI_MAG3] != ELFMAG3)
    {
        print("Invalid ELF magic number.\n");
        return FALSE;
    }

    // Check class (only 32-bit supported here)
    if (header->e_ident[EI_CLASS] != ELFCLASS32)
    {
        print("Unsupported ELF class.\n");
        return FALSE;
    }

    // Check data encoding (only little-endian supported here)
    if (header->e_ident[EI_DATA] != ELFDATA2LSB)
    {
        print("Unsupported ELF data encoding.\n");
        return FALSE;
    }

    // Check file type (only REL and EXEC supported)
    if (header->e_type != ET_EXEC && header->e_type != ET_REL)
    {
        print("Unsupported ELF file type.\n");
        return FALSE;
    }

    // Check machine type (only x86 supported here)
    if (header->e_machine != EM_386)
    {
        print("Unsupported ELF machine type.\n");
        return FALSE;
    }

    // Check version
    if (header->e_ident[EI_VERSION] != EV_CURRENT)
    {
        print("Unsupported ELF version.\n");
        return FALSE;
    }

    return TRUE;
}

void parse_program_headers(ELFHeader *hdr)
{
    if (hdr->e_phoff == 0 || hdr->e_phnum == 0)
    {
        print("No program headers found.\n");
        return;
    }

    print("Parsing program headers:\n");
    ProgramHeader *ph = (ProgramHeader *)((char *)hdr + hdr->e_phoff);
}
static inline void *elf_load_rel(ELFHeader *hdr)
{
    int result;
    result = elf_load_stage1(hdr);
    if (result == ELF_RELOC_ERR)
    {
        ERROR("Unable to load ELF file.\n");
        return 0;
    }
    result = elf_load_stage2(hdr);
    if (result == ELF_RELOC_ERR)
    {
        ERROR("Unable to load ELF file.\n");
        return 0;
    }
    parse_program_headers(hdr);
    return (void *)hdr->e_entry;
}

void *elf_load_file(void *file)
{
    ELFHeader *hdr = (ELFHeader *)file;
    if (!validate_elf(hdr))
    {
        print("ELF File cannot be loaded.\n");
        return;
    }
    switch (hdr->e_type)
    {
    case ET_EXEC:
        need_malloc;
        return 0;
    case ET_REL:
        return elf_load_rel(hdr);
    }
    return 0;
}
static inline SectionHeader *elf_sheader(ELFHeader *hdr)
{
    return (SectionHeader *)((int)hdr + hdr->e_shoff);
}

static inline SectionHeader *elf_section(ELFHeader *hdr, int idx)
{
    return &elf_sheader(hdr)[idx];
}

static inline char *elf_str_table(ELFHeader *hdr)
{
    if (hdr->e_shstrndx == SHN_UNDEF)
        return 0;
    return (char *)hdr + elf_section(hdr, hdr->e_shstrndx)->sh_offset;
}

static inline char *elf_lookup_string(ELFHeader *hdr, int offset)
{
    char *strtab = elf_str_table(hdr);
    if (strtab == 0)
        return 0;
    return strtab + offset;
}
static int elf_get_symval(ELFHeader *hdr, int table, uint32_t idx)
{
    if (table == SHN_UNDEF || idx == SHN_UNDEF)
        return 0;
    SectionHeader *symtab = elf_section(hdr, table);

    uint32_t symtab_entries = symtab->sh_size / symtab->sh_entsize;
    if (idx >= symtab_entries)
    {
        return ELF_RELOC_ERR;
    }

    int symaddr = (int)hdr + symtab->sh_offset;
    SymbolHeader *symbol = &((SymbolHeader *)symaddr)[idx];
    if (symbol->st_shndx == SHN_UNDEF)
    {
        // External symbol, lookup value
        SectionHeader *strtab = elf_section(hdr, symtab->sh_link);
        const char *name = (const char *)hdr + strtab->sh_offset + symbol->st_name;

        extern void *elf_lookup_symbol(const char *name);
        void *target = elf_lookup_symbol(name);

        if (target == 0)
        {
            // Extern symbol not found
            if (ELF32_ST_BIND(symbol->st_info) & STB_WEAK)
            {
                // Weak symbol initialized as 0
                return 0;
            }
            else
            {

                return ELF_RELOC_ERR;
            }
        }
        else
        {
            return (int)target;
        }
    }
    else if (symbol->st_shndx == SHN_ABS)
    {
        // Absolute symbol
        return symbol->st_value;
    }
    else
    {
        // Internally defined symbol
        SectionHeader *target = elf_section(hdr, symbol->st_shndx);
        return (int)hdr + symbol->st_value + target->sh_offset;
    }
}
static int elf_load_stage1(ELFHeader *hdr)
{
    SectionHeader *shdr = elf_sheader(hdr);

    unsigned int i;
    // Iterate over section headers
    for (i = 0; i < hdr->e_shnum; i++)
    {
        SectionHeader *section = &shdr[i];

        // If the section isn't present in the file
        if (section->sh_type == SHT_NOBITS)
        {
            // Skip if it the section is empty
            if (!section->sh_size)
                continue;
            // If the section should appear in memory
            if (section->sh_flags & SHF_ALLOC)
            {

                void *mem = balloc(section->sh_size);
                memset(mem, 0, section->sh_size);
                section->sh_offset = (int)mem - (int)hdr;
            }
        }
    }
    return 0;
}

static int elf_load_stage2(ELFHeader *hdr)
{
    SectionHeader *shdr = elf_sheader(hdr);

    unsigned int i, idx;
    // Iterate over section headers
    for (i = 0; i < hdr->e_shnum; i++)
    {
        SectionHeader *section = &shdr[i];

        // If this is a relocation section
        if (section->sh_type == SHT_REL)
        {
            // Process each entry in the table
            for (idx = 0; idx < section->sh_size / section->sh_entsize; idx++)
            {
                Elf32_Rel *reltab = &((Elf32_Rel *)((int)hdr + section->sh_offset))[idx];
                int result = elf_do_reloc(hdr, reltab, section);
                // On error, display a message and return
                if (result == ELF_RELOC_ERR)
                {
                    print("Failed to relocate symbol.\n");
                    return ELF_RELOC_ERR;
                }
            }
        }
    }
    return 0;
}

static int elf_do_reloc(ELFHeader *hdr, Elf32_Rel *rel, SectionHeader *reltab)
{
    SectionHeader *target = elf_section(hdr, reltab->sh_info);

    int addr = (int)hdr + target->sh_offset;
    int *ref = (int *)(addr + rel->r_offset);
    int symval = 0;
    if (ELF32_R_SYM(rel->r_info) != SHN_UNDEF)
    {
        symval = elf_get_symval(hdr, reltab->sh_link, ELF32_R_SYM(rel->r_info));
        if (symval == ELF_RELOC_ERR)
            return ELF_RELOC_ERR;
    }
    switch (ELF32_R_TYPE(rel->r_info))
    {
    case R_386_NONE:
        // No relocation
        break;
    case R_386_32:
        // Symbol + Offset
        *ref = DO_386_32(symval, *ref);
        break;
    case R_386_PC32:
        // Symbol + Offset - Section Offset
        *ref = DO_386_PC32(symval, *ref, (int)ref);
        break;
    default:
        // Relocation type not supported, display error and return
        return ELF_RELOC_ERR;
    }
    return symval;
}