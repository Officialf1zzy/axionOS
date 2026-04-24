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
           -I./kernel -I./drivers

LDFLAGS := -m elf_i386 -T kernel/linker.ld --oformat binary

# ── Sources & objects ─────────────────────────────────────────
KERNEL_SRC  := $(wildcard kernel/*.c)
DRIVER_SRC  := $(wildcard drivers/*.c)
ALL_SRC     := $(KERNEL_SRC) $(DRIVER_SRC)
OBJS        := $(ALL_SRC:.c=.o)

# ── Output paths ─────────────────────────────────────────────
ISO_DIR     := build/iso
BOOT_DIR    := $(ISO_DIR)/boot
GRUB_DIR    := $(BOOT_DIR)/grub
KERNEL_BIN  := $(BOOT_DIR)/kernel.bin
ISO_OUT     := axionOS.iso

# ── Default target ────────────────────────────────────────────
.PHONY: all run clean

all: $(ISO_OUT)

# ── Compile each .c to .o ─────────────────────────────────────
%.o: %.c
	@echo "  CC  $<"
	@$(CC) $(CFLAGS) -c $< -o $@

# ── Link kernel binary ────────────────────────────────────────
$(KERNEL_BIN): $(OBJS)
	@mkdir -p $(BOOT_DIR)
	@echo "  LD  $@"
	@$(LD) $(LDFLAGS) -o $@ $^

# ── Write grub.cfg if it doesn't exist ───────────────────────
$(GRUB_DIR)/grub.cfg:
	@mkdir -p $(GRUB_DIR)
	@echo "  GEN $@"
	@printf 'set timeout=0\nset default=0\n\nmenuentry "axionOS" {\n    multiboot /boot/kernel.bin\n    boot\n}\n' > $@

# ── Build bootable ISO ────────────────────────────────────────
$(ISO_OUT): $(KERNEL_BIN) $(GRUB_DIR)/grub.cfg
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
	@rm -f $(OBJS) $(KERNEL_BIN) $(ISO_OUT)
	@echo "  Done."
