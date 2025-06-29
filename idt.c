#include <stdint.h>
#include "idt.h"
#include "exceptions.h"
#include "tss.h"
#include "gdt_tss_fill.h"
#include "serial.h"

extern void load_gdt64(void);
extern struct tss64 *get_tss_ptr(void);
extern uint64_t gdt64[];

// Allocate a 4KB double fault stack
__attribute__((aligned(16))) static uint8_t df_stack[4096];

#define IDT_ENTRIES 256

// IDT entry structure
struct idt_entry
{
    uint16_t base_low;
    uint16_t sel;    // Kernel segment selector
    uint8_t ist;     // 3 bits for IST, rest zero
    uint8_t flags;   // Flags (type and privilege level)
    uint16_t base_mid;
    uint32_t base_high;
    uint32_t reserved;
} __attribute__((packed));

// IDT pointer structure
struct idt_ptr
{
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

// Place GDT, IDT in the default data section (higher half, mapped by Limine)
struct idt_entry idt[IDT_ENTRIES];
extern uint64_t gdt64[];

// Declare the IDT and its pointer
struct idt_ptr idtp;

// Minimal default handler (defined below)
extern void default_int_handler(void);

// Load the IDT (defined in assembly)
extern void idt_load(uint64_t);

// Set an entry in the IDT
void idt_set_gate(int num, uint64_t base, uint16_t sel, uint8_t flags, uint8_t ist)
{
    idt[num].base_low = base & 0xFFFF;
    idt[num].base_mid = (base >> 16) & 0xFFFF;
    idt[num].base_high = (base >> 32) & 0xFFFFFFFF;
    idt[num].sel = sel;
    idt[num].ist = ist & 0x7;
    idt[num].flags = flags;
    idt[num].reserved = 0;
}

// Initialize the IDT
void idt_init()
{
    serial_write_string("[DBG] Entering idt_init\r\n");
    serial_write_string("[DBG] gdt64 @ ");
    serial_write_hex((uint64_t)gdt64);
    serial_write_string("\r\n");
    serial_write_string("[DBG] idt @ ");
    serial_write_hex((uint64_t)idt);
    serial_write_string("\r\n");
    serial_write_string("[DBG] tss @ ");
    serial_write_hex((uint64_t)get_tss_ptr());
    serial_write_string("\r\n");
    // Fill in the TSS descriptor in the GDT
    gdt_fill_tss_descriptor(get_tss_ptr(), sizeof(struct tss64));
    serial_write_string("[DBG] TSS descriptor filled\r\n");
    serial_write_string("[DBG] GDT TSS low: ");
    serial_write_hex(gdt64[3]);
    serial_write_string("\r\n");
    serial_write_string("[DBG] GDT TSS high: ");
    serial_write_hex(gdt64[4]);
    serial_write_string("\r\n");
    serial_write_string("[DBG] gdt64_ptr limit: ");
    extern unsigned char gdt64_ptr[];
    serial_write_hex(*(uint16_t*)gdt64_ptr);
    serial_write_string("\r\n[DBG] gdt64_ptr base: ");
    serial_write_hex(*(uint64_t*)(gdt64_ptr + 2));
    serial_write_string("\r\n");
    // Print RSP before calling load_gdt64
    uint64_t rsp_val;
    __asm__ volatile ("mov %%rsp, %0" : "=r"(rsp_val));
    serial_write_string("[DBG] RSP before load_gdt64: ");
    serial_write_hex(rsp_val);
    serial_write_string("\r\n");
    // Inline GDT load and far jump
    extern unsigned char gdt64_ptr[];
    __asm__ volatile (
        "lgdt %0\n"
        "pushq $0x08\n"
        "lea 1f(%%rip), %%rax\n"
        "pushq %%rax\n"
        "lretq\n"
        "1:\n"
        : : "m"(gdt64_ptr) : "memory", "rax"
    );
    serial_write_string("[DBG] GDT loaded\r\n");
    // Install TSS and set IST1 to df_stack
    tss_install(df_stack, sizeof(df_stack));
    serial_write_string("[DBG] TSS installed\r\n");
    idtp.limit = (sizeof(struct idt_entry) * IDT_ENTRIES) - 1;
    idtp.base = (uint64_t)&idt;
    serial_write_string("[DBG] idtp.base = ");
    serial_write_hex(idtp.base);
    serial_write_string("\r\n");

    // Install proper exception handlers for vectors 0-31
    extern void isr0(void), isr1(void), isr2(void), isr3(void), isr4(void), isr5(void), isr6(void), isr7(void);
    extern void isr8(void), isr9(void), isr10(void), isr11(void), isr12(void), isr13(void), isr14(void), isr15(void);
    extern void isr16(void), isr17(void), isr18(void), isr19(void), isr20(void), isr21(void), isr22(void), isr23(void);
    extern void isr24(void), isr25(void), isr26(void), isr27(void), isr28(void), isr29(void), isr30(void), isr31(void);
    void (*isr_table[32])(void) = {
        isr0, isr1, isr2, isr3, isr4, isr5, isr6, isr7,
        isr8, isr9, isr10, isr11, isr12, isr13, isr14, isr15,
        isr16, isr17, isr18, isr19, isr20, isr21, isr22, isr23,
        isr24, isr25, isr26, isr27, isr28, isr29, isr30, isr31
    };
    for (int i = 0; i < 32; i++) {
        // For double fault (vector 8), set IST to 1
        uint8_t ist = (i == 8) ? 1 : 0;
        uint8_t flags = 0x8E; // type=0xE, present=1, DPL=0
        idt_set_gate(i, (uint64_t)isr_table[i], 0x08, flags, ist);
    }

    // Install timer IRQ handler at vector 32
    extern void timer_handler_asm(void);
    idt_set_gate(32, (uint64_t)timer_handler_asm, 0x08, 0x8E, 0);

    // Debug: Print IDT[32] fields
    serial_write_string("[DBG] idt[32] base: ");
    serial_write_hex(((uint64_t)idt[32].base_high << 32) | ((uint64_t)idt[32].base_mid << 16) | idt[32].base_low);
    serial_write_string("\r\n[DBG] idt[32] sel: ");
    serial_write_hex(idt[32].sel);
    serial_write_string("\r\n[DBG] idt[32] flags: ");
    serial_write_hex(idt[32].flags);
    serial_write_string("\r\n[DBG] idt[32] ist: ");
    serial_write_hex(idt[32].ist);
    serial_write_string("\r\n");

    // Set all other entries to a minimal default handler
    for (int i = 33; i < IDT_ENTRIES; i++)
    {
        idt_set_gate(i, (uint64_t)default_int_handler, 0x08, 0x8E, 0);
    }

    // Load the IDT
    idt_load((uint64_t)&idtp);
    serial_write_string("[DBG] Leaving idt_init\r\n");
}

// Minimal default handler for all interrupts (64-bit, prints and halts)
__attribute__((naked)) void default_int_handler(void) {
    __asm__ volatile (
        "push %rax\n"
        "mov $0x3F8, %dx\n" // COM1
        "mov $'!', %al\n"
        "out %al, %dx\n"
        "pop %rax\n"
        "cli\n"
        "hlt\n"
        "jmp .\n"
    );
}