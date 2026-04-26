# AxionOS Makefile
# Usage:
#   make        — build the ISO
#   make run    — build and launch in QEMU
#   make clean  — remove all build artefacts

# ── Toolchain ────────────────────────────────────────────────
CC      := gcc
LD      := ld
MKISO   := grub-mkrescue
QEMU    := qemu-system-i386

# ── Flags ────────────────────────────────────────────────────
CFLAGS  := -m32 -std=c99 \
           -ffreestanding -fno-stack-protector \
           -fno-builtin -nostdlib -nostdinc \
           -Wall -Wextra \
           -I./kernel -I./drivers -I./include -I./userland

LDFLAGS := -m elf_i386 -T kernel/linker.ld

# ── Sources & objects ─────────────────────────────────────────
KERNEL_SRC  := $(wildcard kernel/*.c)
DRIVER_SRC  := $(wildcard drivers/*.c)
ASM_SRC     := $(wildcard drivers/*.asm)
ALL_SRC     := $(KERNEL_SRC) $(DRIVER_SRC)
OBJS        := $(ALL_SRC:.c=.o) $(ASM_SRC:.asm=.o)

# ── Output paths ─────────────────────────────────────────────
ISO_DIR     := build/iso
BOOT_DIR    := $(ISO_DIR)/boot
GRUB_DIR    := $(BOOT_DIR)/grub
KERNEL_ELF  := $(BOOT_DIR)/kernel.elf
ISO_OUT     := axionOS.iso

# ── Default target ────────────────────────────────────────────
.PHONY: all run clean

all: $(ISO_OUT)

# ── Compile each .asm to .o ───────────────────────────────────
%.o: %.asm
	@echo "  NASM $<"
	@nasm -f elf32 $< -o $@

# ── Compile each .c to .o ─────────────────────────────────────
%.o: %.c
	@echo "  CC  $<"
	@$(CC) $(CFLAGS) -c $< -o $@

# ── Link kernel ELF ───────────────────────────────────────────
$(KERNEL_ELF): $(OBJS)
	@mkdir -p $(BOOT_DIR)
	@echo "  LD  $@"
	@$(LD) $(LDFLAGS) -o $@ $^

# ── Write grub.cfg ────────────────────────────────────────────
$(GRUB_DIR)/grub.cfg:
	@mkdir -p $(GRUB_DIR)
	@echo "  GEN $@"
	@printf 'set timeout=0\nset default=0\n\nmenuentry "axionOS" {\n    multiboot2 /boot/kernel.elf\n    boot\n}\n' > $@

# ── Build bootable ISO ────────────────────────────────────────
$(ISO_OUT): $(KERNEL_ELF) $(GRUB_DIR)/grub.cfg
	@echo "  ISO $@"
	@$(MKISO) -o $@ $(ISO_DIR)
	@echo ""
	@echo "  Build complete → $(ISO_OUT)"

# ── Launch in QEMU ────────────────────────────────────────────
run: $(ISO_OUT)
	@echo "  Launching axionOS in QEMU..."
	@$(QEMU) -cdrom $(ISO_OUT) -m 32M

# ── Clean ─────────────────────────────────────────────────────
clean:
	@echo "  Cleaning..."
	@rm -f $(OBJS) $(KERNEL_ELF) $(ISO_OUT)
	@rm -rf $(ISO_DIR)
	@echo "  Done."
