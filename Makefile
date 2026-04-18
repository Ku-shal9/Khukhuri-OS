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

ASM_OBJ        := $(BUILD_DIR)/kernel_entry.o \
                  $(BUILD_DIR)/interrupts.o
C_OBJS         := $(BUILD_DIR)/kernel.o \
                  $(BUILD_DIR)/screen.o \
                  $(BUILD_DIR)/idt.o \
                  $(BUILD_DIR)/isr.o \
                  $(BUILD_DIR)/keyboard.o \
                  $(BUILD_DIR)/timer.o \
                  $(BUILD_DIR)/memory.o \
                  $(BUILD_DIR)/process.o \
                  $(BUILD_DIR)/fat.o \
                  $(BUILD_DIR)/filesystem.o \
                  $(BUILD_DIR)/kernel_shell.o

.PHONY: all build iso run run-debug clean check-tools

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

$(BUILD_DIR)/kernel_entry.o: kernel/kernel_entry.asm | $(BUILD_STAMP)
	$(AS) -f elf32 $< -o $@

$(BUILD_DIR)/interrupts.o: kernel/interrupts.asm | $(BUILD_STAMP)
	$(AS) -f elf32 $< -o $@

$(BUILD_DIR)/kernel.o: kernel/kernel.c | $(BUILD_STAMP)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/screen.o: drivers/screen.c | $(BUILD_STAMP)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/idt.o: kernel/idt.c | $(BUILD_STAMP)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/isr.o: kernel/isr.c | $(BUILD_STAMP)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/keyboard.o: drivers/keyboard.c | $(BUILD_STAMP)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/timer.o: drivers/timer.c | $(BUILD_STAMP)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/memory.o: memory/memory.c | $(BUILD_STAMP)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/process.o: process/process.c | $(BUILD_STAMP)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/fat.o: filesystem/fat.c | $(BUILD_STAMP)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/filesystem.o: filesystem/filesystem.c | $(BUILD_STAMP)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/kernel_shell.o: shell/kernel_shell.c | $(BUILD_STAMP)
	$(CC) $(CFLAGS) -c $< -o $@

$(KERNEL_ELF): $(ASM_OBJ) $(C_OBJS) linker.ld
	$(CC) $(LDFLAGS) -o $@ $(ASM_OBJ) $(C_OBJS) -lgcc

$(KERNEL_BIN): $(KERNEL_ELF)
	$(ARCH)-objcopy -O binary $< $@

build: $(KERNEL_BIN)

iso: build
	mkdir -p $(ISO_DIR)/boot/grub
	cp $(KERNEL_ELF) $(ISO_DIR)/boot/kernel.elf
	cp boot/grub/grub.cfg $(ISO_DIR)/boot/grub/grub.cfg
	$(GRUB_MKRESCUE) -o $(ISO_IMAGE) $(ISO_DIR)

run: iso
	$(QEMU) -cdrom $(ISO_IMAGE) -no-reboot -no-shutdown -monitor none -serial none -parallel none

run-debug: iso
	$(QEMU) -cdrom $(ISO_IMAGE) -no-reboot -no-shutdown -monitor none -serial none -parallel none -d int,cpu_reset -D $(BUILD_DIR)/qemu.log

clean:
	rm -rf $(BUILD_DIR)