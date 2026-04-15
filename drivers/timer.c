#include "timer.h"

unsigned int tick = 0;

static void outb(unsigned short port, unsigned char value) {
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static unsigned char inb(unsigned short port) {
    unsigned char result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

void timer_init() {
    unsigned int divisor = 1193180 / 100;
    outb(0x43, 0x36);
    outb(0x40, divisor & 0xFF);
    outb(0x40, (divisor >> 8) & 0xFF);

    unsigned char mask = inb(0x21);
    mask &= (unsigned char)~0x01;
    outb(0x21, mask);
}

void timer_handler() {
    tick++;
}

void delay(int ticks) {
    unsigned int start = tick;
    while (tick < start + ticks);
}

unsigned int timer_ticks() {
    return tick;
}