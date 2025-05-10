/*
 * bootboot.h
 * 
 * BOOTBOOT Protocol specific header for x86_64-efi
 */

#include <stddef.h>
#include <stdint.h>
#ifndef _BOOTBOOT_H_
#define _BOOTBOOT_H_

// Standard types are now defined in kernel.c

#ifdef  __cplusplus
extern "C" {
#endif

/* 
 * Boot protocol level. 
 * 1 = Static, 2 = Minimal, 3 = Dynamic
 */
#define PROTOCOL_MINIMAL    1
#define PROTOCOL_STATIC     2
#define PROTOCOL_DYNAMIC    3
#define PROTOCOL_REVISION   1

/* 
 * Bootloader information structure 
 */
typedef struct {
    /* First 64 bytes are platform independent */
    uint8_t     magic[4];                   /* 'BOOT' magic */
    uint32_t    size;                       /* Length of bootboot structure, minimum 128 */
    uint8_t     protocol;                   /* 1,2,3 for static, minimal, dynamic */
    uint8_t     revision;                   /* Spec revision, currently 1 */
    uint8_t     arch;                       /* Architecture: 0=x86, 1=aarch64, ... */
    uint8_t     reserved_0;
    uint64_t    initrd_ptr;                 /* Physical address of initrd */
    uint64_t    initrd_size;                /* Size of initrd */
    uint64_t    fb_ptr;                     /* Physical address of framebuffer */
    uint32_t    fb_size;                    /* Size of framebuffer in bytes */
    uint32_t    fb_width;                   /* Width and height of framebuffer */
    uint32_t    fb_height;
    uint32_t    fb_scanline;                /* Scanline length */

    /* The rest can be architecture specific */
    uint8_t     reserved_1[8];
    uint64_t    x86_64_acpi_ptr;            /* ACPI table pointer */
    uint64_t    x86_64_smbi_ptr;            /* SMBIOS table pointer */
    uint64_t    x86_64_efi_ptr;             /* EFI system table pointer */
    uint64_t    x86_64_mps_ptr;             /* MPS table pointer */
    uint64_t    x86_64_pcie_ptr;            /* PCIe MMIO config space pointer */
    uint64_t    x86_64_mm_ptr;              /* Memory map */
    uint64_t    x86_64_mm_size;             /* Memory map size */
    uint64_t    reserved_2[2];
} __attribute__((packed)) BOOTBOOT;

/*
 * Memory map entry
 */
typedef struct {
    uint64_t    ptr;
    uint64_t    size;
    uint32_t    type;                       /* Type: 1=free, 2=reserved, etc. */
    uint32_t    reserved_0;
} __attribute__((packed)) MMapEnt;

/* 
 * MMap entry types 
 */
#define MMAP_USED     0                     /* Used by the bootloader */
#define MMAP_FREE     1                     /* Usable memory */
#define MMAP_ACPI     2                     /* ACPI memory, volatile and non-volatile */
#define MMAP_MMIO     3                     /* Memory mapped IO region */

/* 
 * Framebuffer pixel format 
 */
#define FB_ARGB   0
#define FB_RGBA   1
#define FB_ABGR   2
#define FB_BGRA   3

/* 
 * Environment variable structure (after the environment header) 
 */
typedef struct {
    uint8_t     key[64];                    /* Key name */
    uint8_t     value[192];                 /* Value */
} __attribute__((packed)) EnvironmentVar;

#ifdef  __cplusplus
}
#endif

#endif