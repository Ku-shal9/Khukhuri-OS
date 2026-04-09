ARCH           := i686-elf
CC             := $(ARCH)-gcc
LD             := $(ARCH)-ld
AS             := nasm
GRUB_MKRESCUE  := grub-mkrescue
QEMU           := qemu-system-i386

CFLAGS         := -ffreestanding -O2 -Wall -Wextra -Iinclude
LDFLAGS        := -T linker.ld -ffreestanding -O2 -nostdlib

BUILD_DIR      := build
BUILD_STAMP    := $(BUILD_DIR)/.dir
ISO_DIR        := $(BUILD_DIR)/iso
KERNEL_ELF     := $(BUILD_DIR)/kernel.elf
KERNEL_BIN     := $(BUILD_DIR)/kernel.bin
ISO_IMAGE      := $(BUILD_DIR)/myos.iso

ASM_OBJ        := $(BUILD_DIR)/kernel_entry.o
C_OBJS         := $(BUILD_DIR)/kernel.o \
                  $(BUILD_DIR)/screen.o \
                  $(BUILD_DIR)/memory.o \
                  $(BUILD_DIR)/process.o

.PHONY: all build iso run clean check-tools

all: build

check-tools:
	@command -v $(CC) >/dev/null || (echo "Missing $(CC). Install cross-compiler."; exit 1)
	@command -v $(LD) >/dev/null || (echo "Missing $(LD). Install cross-linker."; exit 1)
	@command -v $(AS) >/dev/null || (echo "Missing nasm."; exit 1)
	@command -v $(GRUB_MKRESCUE) >/dev/null || (echo "Missing grub-mkrescue."; exit 1)
	@command -v $(QEMU) >/dev/null || (echo "Missing qemu-system-i386."; exit 1)

$(BUILD_STAMP):
	mkdir -p $(BUILD_DIR)
	touch $(BUILD_STAMP)

$(ASM_OBJ): kernel/kernel_entry.asm | $(BUILD_STAMP)
	$(AS) -f elf32 $< -o $@

$(BUILD_DIR)/kernel.o: kernel/kernel.c | $(BUILD_STAMP)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/screen.o: drivers/screen.c | $(BUILD_STAMP)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/memory.o: memory/memory.c | $(BUILD_STAMP)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/process.o: process/process.c | $(BUILD_STAMP)
	$(CC) $(CFLAGS) -c $< -o $@

$(KERNEL_ELF): $(ASM_OBJ) $(C_OBJS) linker.ld
	$(CC) $(LDFLAGS) -o $@ $(ASM_OBJ) $(C_OBJS) -lgcc

$(KERNEL_BIN): $(KERNEL_ELF)
	$(ARCH)-objcopy -O binary $< $@

build: $(KERNEL_BIN)

iso: build
	mkdir -p $(ISO_DIR)/boot/grub
	cp $(KERNEL_BIN) $(ISO_DIR)/boot/kernel.bin
	cp boot/grub/grub.cfg $(ISO_DIR)/boot/grub/grub.cfg
	$(GRUB_MKRESCUE) -o $(ISO_IMAGE) $(ISO_DIR)

run: iso
	$(QEMU) -cdrom $(ISO_IMAGE)

clean:
	rm -rf $(BUILD_DIR)