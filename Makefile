# Makefile for BOOTBOOT x86_64-EFI kernel

# Compiler and tools
CC = gcc
LD = ld
OBJCOPY = objcopy

# Architecture-specific flags
ARCH_FLAGS = -m64 -mno-red-zone

# Compiler flags (applies to both C and assembly via GCC)
CFLAGS = $(ARCH_FLAGS) -ffreestanding -O2 -Wall -Wextra -Werror \
         -fno-exceptions -fno-stack-protector -nostdlib \
         -fno-builtin -mno-mmx -mno-sse -mno-sse2 -DKERNEL_BASE=0x100000

# Linker flags (specify entry point if required)
LDFLAGS = -nostdlib -T linker.ld -z noexecstack --nmagic

# Source files
C_SOURCES = $(wildcard *.c)
ASM_SOURCES = $(wildcard *.s)

# Object files
C_OBJECTS = $(C_SOURCES:.c=.o)
ASM_OBJECTS = $(ASM_SOURCES:.s=.o)
OBJECTS = $(C_OBJECTS) $(ASM_OBJECTS)

# The kernel executable
KERNEL = kernel.elf

# Default target
all: $(KERNEL)

# Rule to build the kernel
$(KERNEL): $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^
	$(OBJCOPY) -O binary $(KERNEL) kernel.bin

# Compile C files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Assemble ASM files with GCC (ensures preprocessor runs if needed)
%.o: %.s
	$(CC) $(CFLAGS) -x assembler -c $< -o $@

# Clean up
clean:
	rm -f $(KERNEL) kernel.bin $(OBJECTS)

# QEMU testing
qemu: $(KERNEL)
	qemu-system-x86_64 -M q35 -m 128 -kernel kernel.bin

.PHONY: all clean qemu