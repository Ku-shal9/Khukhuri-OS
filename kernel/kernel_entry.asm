global _start
extern kernel_main

; Multiboot header for GRUB.
section .multiboot
align 4
    dd 0x1BADB002                ; magic
    dd 0x00000000                ; flags
    dd -(0x1BADB002 + 0x00000000); checksum

section .text
_start:
    call kernel_main
    cli
.halt:
    hlt
    jmp .halt