#include "scheduler.h"
#include "exe.h"

#define NULL 0
uint32_t validate_elf(ELFHeader *header)
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

page_directory_entry_t *parse_program_headers(ELFHeader *hdr)
{
    if (hdr->e_phoff == 0 || hdr->e_phnum == 0)
    {
        print("No program headers found.\n");
        return;
    }
    page_directory_entry_t *n_pd = create_page_directory();
    switch_page_directory(virt_to_phys(n_pd));
    ProgramHeader *ph = (ProgramHeader *)((char *)hdr + hdr->e_phoff);

    for (int i = 0; i < hdr->e_phnum; i++)
    {
        if (ph[i].p_type == PT_LOAD)
        {
            palloc(&ph[i], (void *)hdr, n_pd);
        }
    }
    return n_pd;
}
void *elf_load_rel(ELFHeader *hdr)
{
    uint32_t result1;
    result1 = elf_load_stage1(hdr);
    if (result1 == ELF_RELOC_ERR)
    {
        print("Unable to load ELF file.\n");
        return 0;
    }
    uint32_t result2;
    result2 = elf_load_stage2(hdr);
    if (result2 == ELF_RELOC_ERR)
    {
        print("Unable to load ELF file.\n");
        return 0;
    }
    parse_program_headers(hdr);
    return (void *)hdr->e_entry;
}
uint32_t k_stack;
uint32_t current = 0xf0000000 - 0x400000 - 0xf;
struct kthread *elf_load_file(void *file)
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
        if (!hdr->e_entry)
        {
            print("Invalid entry point.\n");
            return NULL;
        }
        uint32_t esp = current;
        uint32_t heap = 0x08048000;
        k_stack = get_esp();
        page_directory_entry_t *sf = parse_program_headers(hdr);
        unsigned long Hpdindex = heap >> 22;
        sf[Hpdindex].page_size = 1;
        sf[Hpdindex].present = 1;
        sf[Hpdindex].rw = 1;
        sf[Hpdindex].user = 1;
        sf[Hpdindex].table_addr = (uint32_t)balloc(&pbud, 0x1000, 1) >> 12;
        unsigned long Spdindex = (esp >> 22);
        sf[Spdindex].page_size = 1;
        sf[Spdindex].present = 1;
        sf[Spdindex].rw = 1;
        sf[Spdindex].user = 1;
        current -= 0x400000;
        if (scheduler_enabled)
        {
            return init_task(virt_to_phys(sf), esp, hdr->e_entry, 0, 1);
        }
        jump_usermode(hdr->e_entry, virt_to_phys(sf), esp);
    case ET_REL:
        kprintf("os isnt suporrting relocation!");
        return (struct kthread *)0;
    }
    return 0;
}
SectionHeader *elf_sheader(ELFHeader *hdr)
{
    return (SectionHeader *)((int)hdr + hdr->e_shoff);
}

SectionHeader *elf_section(ELFHeader *hdr, int idx)
{
    return &elf_sheader(hdr)[idx];
}

char *elf_str_table(ELFHeader *hdr)
{
    if (hdr->e_shstrndx == SHN_UNDEF)
        return 0;
    return (char *)hdr + elf_section(hdr, hdr->e_shstrndx)->sh_offset;
}

char *elf_lookup_string(ELFHeader *hdr, int offset)
{
    char *strtab = elf_str_table(hdr);
    if (strtab == 0)
        return 0;
    return strtab + offset;
}
uint32_t elf_get_symval(ELFHeader *hdr, int table, uint32_t idx)
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

        void *target = elf_lookup_symbol(name, hdr);

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
uint32_t elf_load_stage1(ELFHeader *hdr)
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

                void *mem = kalloc(section->sh_size);
                memset(mem, 0, section->sh_size);
                section->sh_offset = (int)mem - (int)hdr;
            }
        }
    }
    return 0;
}

uint32_t elf_load_stage2(ELFHeader *hdr)
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
void *elf_lookup_symbol(const char *name, ELFHeader *hdr)
{
    if (!name || name[0] == '\0')
    {
        print("Invalid symbol name.\n");
        return NULL;
    }

    // Iterate through the section headers
    SectionHeader *shdr = elf_sheader(hdr);

    for (int i = 0; i < hdr->e_shnum; i++)
    {
        SectionHeader *section = &shdr[i];

        // Check for the symbol table section
        if (section->sh_type == SHT_SYMTAB)
        {
            // Symbol table found, iterate through its entries
            int symtab_entries = section->sh_size / section->sh_entsize;
            SymbolHeader *symbol = (SymbolHeader *)((int)hdr + section->sh_offset);

            // Iterate through each symbol in the symbol table
            for (int idx = 0; idx < symtab_entries; idx++)
            {
                const char *symbol_name = append_strings((const char *)hdr, elf_lookup_string(hdr, symbol[idx].st_name));

                if (strcmp(name, symbol_name, strlen(name)) == 0)
                {
                    // Found the symbol, handle its binding and return the address

                    if (symbol[idx].st_shndx == SHN_UNDEF)
                    {
                        // External symbol, perform lookup
                        SectionHeader *strtab = elf_section(hdr, section->sh_link); // Get the string table
                        const char *external_name = elf_lookup_string(hdr, symbol[idx].st_name);
                        void *target = elf_lookup_symbol(external_name, hdr);

                        if (target == NULL)
                        {
                            // If external symbol isn't found, check if weak
                            if (ELF32_ST_BIND(symbol[idx].st_info) == STB_WEAK)
                            {
                                return NULL; // Weak symbols resolve to 0
                            }
                            else
                            {
                                return (void *)ELF_RELOC_ERR; // Undefined symbol, error
                            }
                        }
                        else
                        {
                            return target; // Return the symbol's address
                        }
                    }
                    else if (symbol[idx].st_shndx == SHN_ABS)
                    {
                        // Absolute symbol
                        return (void *)(symbol[idx].st_value);
                    }
                    else
                    {
                        // Internal symbol, resolve address within the section
                        SectionHeader *target_section = elf_section(hdr, symbol[idx].st_shndx);
                        return (void *)((int)hdr + symbol[idx].st_value + target_section->sh_offset);
                    }
                }
            }
        }
    }

    // If the symbol wasn't found
    print("Symbol not found: ");
    print(name);
    return (void *)ELF_RELOC_ERR;
}
uint32_t elf_do_reloc(ELFHeader *hdr, Elf32_Rel *rel, SectionHeader *reltab)
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
