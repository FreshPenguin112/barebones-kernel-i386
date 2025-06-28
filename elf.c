#include "tarfs.h"
#include "string_utils.h"
#include <stdint.h>

#define EI_NIDENT 16

typedef struct {
    unsigned char e_ident[EI_NIDENT];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint64_t e_entry;
    uint64_t e_phoff;
    uint64_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} Elf64_Ehdr;

typedef struct {
    uint32_t p_type;
    uint32_t p_flags;
    uint64_t p_offset;
    uint64_t p_vaddr;
    uint64_t p_paddr;
    uint64_t p_filesz;
    uint64_t p_memsz;
    uint64_t p_align;
} Elf64_Phdr;

typedef void (*entry_fn_t)(void);

extern volatile int user_program_exited;

static void (*elf_exit_callback)(void) = 0;

void elf_exit() {
    user_program_exited = 1;
}

static uint8_t user_stack[0x4000] __attribute__((aligned(16)));

int elf_run(const char *filename) {
    unsigned int size;
    const char *data = tarfs_cat(filename, &size);
    if (!data) return -1;
    const Elf64_Ehdr *ehdr = (const Elf64_Ehdr *)data;
    if (ehdr->e_ident[0] != 0x7f || ehdr->e_ident[1] != 'E' || ehdr->e_ident[2] != 'L' || ehdr->e_ident[3] != 'F') return -2;
    if (ehdr->e_ident[4] != 2 || ehdr->e_ident[5] != 1 || ehdr->e_type != 2) return -3; // 64-bit, little-endian, executable
    for (int i = 0; i < ehdr->e_phnum; i++) {
        const Elf64_Phdr *ph = (const Elf64_Phdr *)(data + ehdr->e_phoff + i * ehdr->e_phentsize);
        if (ph->p_type != 1) continue; // PT_LOAD
        char *dest = (char *)ph->p_vaddr;
        const char *src = data + ph->p_offset;
        for (uint64_t j = 0; j < ph->p_filesz; j++) dest[j] = src[j];
        for (uint64_t j = ph->p_filesz; j < ph->p_memsz; j++) dest[j] = 0;
    }
    entry_fn_t entry = (entry_fn_t)ehdr->e_entry;
    elf_exit_callback = elf_exit;
    user_program_exited = 0;
    // Set up a separate user stack (top of user_stack, minus 8 for call alignment)
    __asm__ volatile (
        "mov %[ustack_top], %%rsp\n"
        : : [ustack_top] "r" (user_stack + sizeof(user_stack) - 8)
        : "rsp"
    );
    entry();
    user_program_exited = 1;
    return 0;
}