#include "screen.h"
#include "idt.h"
#include "keyboard.h"
#include "timer.h"
#include "filesystem.h"
#include "khukhuri_shell.h"

void kernel_main() {
    clear_screen();
    print("Kernel Boot Successful!");
    print("Milestone 2 Start\n");

    idt_init();
    keyboard_init();
    timer_init();
    fs_init();
    __asm__ volatile("sti");

    print("Interrupt system ready\n");
    khukhuri_shell_init();
    print("Keyboard input ready (set1/set2)\n");

    while (1) {
        keyboard_poll();
    }
}