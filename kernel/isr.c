#include "keyboard.h"
#include "timer.h"
#include "mouse.h"

static void outb(unsigned short port, unsigned char value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static void pic_send_eoi(int interrupt) {
    int irq = interrupt - 32;
    if (irq >= 8) {
        outb(0xA0, 0x20);
    }
    outb(0x20, 0x20);
}

void isr_handler(int interrupt) {
    if (interrupt == 32) {
        timer_handler();
        pic_send_eoi(interrupt);
    } else if (interrupt == 33) {
        keyboard_handler();
        pic_send_eoi(interrupt);
    } else if (interrupt == 44) {
        mouse_handler();
        pic_send_eoi(interrupt);
    }
}