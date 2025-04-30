#include "tarfs.h"
#include "string_utils.h"
#include <stdint.h>

#define EI_NIDENT 16

typedef struct
{
    unsigned char e_ident[EI_NIDENT];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint32_t e_entry;
    uint32_t e_phoff;
    uint32_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} Elf32_Ehdr;

typedef struct
{
    uint32_t p_type;
    uint32_t p_offset;
    uint32_t p_vaddr;
    uint32_t p_paddr;
    uint32_t p_filesz;
    uint32_t p_memsz;
    uint32_t p_flags;
    uint32_t p_align;
} Elf32_Phdr;

typedef void (*entry_fn_t)(void);

extern volatile int user_program_exited;

static void (*elf_exit_callback)(void) = 0;

void elf_exit()
{
    user_program_exited = 1;
    // Return control to elf_run
    // If you want to use setjmp/longjmp, you can do it here
}

int elf_run(const char *filename)
{
    unsigned int size;
    const char *data = tarfs_cat(filename, &size);
    if (!data)
        return -1;

    const Elf32_Ehdr *ehdr = (const Elf32_Ehdr *)data;
    if (ehdr->e_ident[0] != 0x7f || ehdr->e_ident[1] != 'E' ||
        ehdr->e_ident[2] != 'L' || ehdr->e_ident[3] != 'F')
    {
        return -2;
    }

    // Only support 32-bit, little-endian, executable
    if (ehdr->e_ident[4] != 1 || ehdr->e_ident[5] != 1 || ehdr->e_type != 2)
    {
        return -3;
    }

    // Load program headers
    for (int i = 0; i < ehdr->e_phnum; i++)
    {
        const Elf32_Phdr *ph = (const Elf32_Phdr *)(data + ehdr->e_phoff + i * ehdr->e_phentsize);
        if (ph->p_type != 1)
            continue; // PT_LOAD
        // Copy segment to memory
        char *dest = (char *)ph->p_vaddr;
        const char *src = data + ph->p_offset;
        for (uint32_t j = 0; j < ph->p_filesz; j++)
            dest[j] = src[j];
        // Zero out .bss
        for (uint32_t j = ph->p_filesz; j < ph->p_memsz; j++)
            dest[j] = 0;
    }

    // Jump to entry point
    entry_fn_t entry = (entry_fn_t)ehdr->e_entry;

    elf_exit_callback = elf_exit;

    user_program_exited = 0;
    entry();                 // Call the user program's entry point
    user_program_exited = 1; // Mark as exited if it returns

    return 0;
}