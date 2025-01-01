#include "exe.h"
#include "std.h"
#include "process.h"
int validate_elf(ELFHeader *header)
{
    if (header->magic != ELF_MAGIC)
    {
        return 0; // Not a valid ELF file
    }
    if (header->eclass != ELF_CLASS_32 && header->eclass != ELF_CLASS_64)
    {
        return 0; // Unsupported ELF class
    }
    if (header->data != ELF_DATA_LSB)
    {
        return 0; // Only support little-endian for now
    }
    return 1; // Valid ELF file
}
