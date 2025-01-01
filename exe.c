#include "exe.h"
#include "std.h"
#include "process.h"
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
        ERROR("Invalid ELF magic number.\n");
        return FALSE;
    }

    // Check class (only 32-bit supported here)
    if (header->e_ident[EI_CLASS] != ELFCLASS32)
    {
        ERROR("Unsupported ELF class.\n");
        return FALSE;
    }

    // Check data encoding (only little-endian supported here)
    if (header->e_ident[EI_DATA] != ELFDATA2LSB)
    {
        ERROR("Unsupported ELF data encoding.\n");
        return FALSE;
    }

    // Check file type (only REL and EXEC supported)
    if (header->e_type != ET_EXEC && header->e_type != ET_REL)
    {
        ERROR("Unsupported ELF file type.\n");
        return FALSE;
    }

    // Check machine type (only x86 supported here)
    if (header->e_machine != EM_386)
    {
        ERROR("Unsupported ELF machine type.\n");
        return FALSE;
    }

    // Check version
    if (header->e_ident[EI_VERSION] != EV_CURRENT)
    {
        ERROR("Unsupported ELF version.\n");
        return TRUE;
    }

    return TRUE;
}
