#include "screen.h"
#include "printf.h"
#include "idt.h"
#include "keyboard.h"
#include "timer.h"
#include "filesystem.h"
#include "console.h"
#include "mouse.h"

static void show_boot_banner(void) {
    printf("\033[1;36m");
    printf(
"      / \\                      / \\\n"
"     /   \\                    /   \\\n"
"    /  /| \\                  / | \\ \\\n"
"   /  /_|  \\                /  |_ \\ \\\n"
"  /  /      \\              /      \\  \\\n"
" (  (        \\            /        )  )\n"
"  \\  \\        \\    __    /        /  /\n"
"   \\  \\        \\  /  \\  /        /  /\n"
"    \\  \\________\\/____\\/________/  /\n"
"     \\________  /      \\  ________/\n"
"             / /        \\ \\\n"
"            / /          \\ \\\n"
"           (  )          (  )\n"
"            --            --\n"
);

    printf("+==========================================================+\n");
    printf("|            K   H   U   K   H   U   R   I   -   O S        |\n");
    printf("+==========================================================+\n");
    printf("\033[1;33m");
    printf("|  \033[1;31m*\033[1;33m  Secure boot gate enabled (PIN + shell keyword)   \033[1;31m*\033[1;33m  |\n");
    printf("+==========================================================+\n");
    printf("\033[0m\n");
    printf("\033[1;32mKernel online.\033[0m \033[1;37mScroll:\033[0m PgUp/PgDn or mouse wheel if supported.\n\n");
}

void kernel_main(void) {
    clear_screen();
    show_boot_banner();

    idt_init();
    keyboard_init();
    timer_init();
    mouse_init();
    fs_init();

    console_init();

    __asm__ volatile("sti");

    while (1) {
        __asm__ volatile("hlt");
    }
}
