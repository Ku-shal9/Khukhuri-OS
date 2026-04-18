#include "idt.h"

struct idt_entry {
    unsigned short base_low;
    unsigned short sel;
    unsigned char always0;
    unsigned char flags;
    unsigned short base_high;
} __attribute__((packed));

struct idt_ptr {
    unsigned short limit;
    unsigned int base;
} __attribute__((packed));

struct idt_entry idt[256];
struct idt_ptr idtp;

extern void idt_load();
extern void irq0();
extern void irq1();

static void outb(unsigned short port, unsigned char value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static unsigned char inb(unsigned short port) {
    unsigned char result;
    __asm__ volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

static void io_wait(void) {
    __asm__ volatile ("outb %%al, $0x80" : : "a"(0));
}

static void remap_pic(void) {
    unsigned char master_mask = inb(0x21);
    unsigned char slave_mask = inb(0xA1);

    outb(0x20, 0x11);
    io_wait();
    outb(0xA0, 0x11);
    io_wait();
    outb(0x21, 0x20);
    io_wait();
    outb(0xA1, 0x28);
    io_wait();
    outb(0x21, 0x04);
    io_wait();
    outb(0xA1, 0x02);
    io_wait();
    outb(0x21, 0x01);
    io_wait();
    outb(0xA1, 0x01);
    io_wait();

    outb(0x21, master_mask);
    outb(0xA1, slave_mask);
}

void idt_set_gate(int num, unsigned long base) {
    unsigned short cs_selector;
    __asm__ volatile ("mov %%cs, %0" : "=r"(cs_selector));

    idt[num].base_low = base & 0xFFFF;
    idt[num].base_high = (base >> 16) & 0xFFFF;

    idt[num].sel = cs_selector;
    idt[num].always0 = 0;
    idt[num].flags = 0x8E;
}

void idt_init() {
    idtp.limit = sizeof(struct idt_entry) * 256 - 1;
    idtp.base = (unsigned int)&idt;

    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, 0);
    }

    remap_pic();
    idt_set_gate(32, (unsigned long)irq0);
    idt_set_gate(33, (unsigned long)irq1);
    idt_load();
}