#include "keyboard.h"
#include "khukhuri_shell.h"

static const char keymap[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',
    '9', '0', '-', '=', '\b', '\t', 'q', 'w', 'e', 'r',
    't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
    '\'', '`', 0, '\\', 'z', 'x', 'c', 'v', 'b', 'n',
    'm', ',', '.', '/', 0, '*', 0, ' ', 0,  0,
    0,  0,   0,   0,   0,   0,   0,  0,   0,  0,
    0,  0,   0,   0,   '-', 0,   0,  0,   '+', 0,
    0,  0,   0,   0,   0,   0,   0,  0,   0,  0,
    0,  0,   0,   0,   0,   0,   0,  0,   0,  0,
    0,  0,   0,   0,   0,   0,   0,  0,   0,  0,
    0,  0,   0,   0,   0,   0,   0,  0
};

static unsigned char inb(unsigned short port) {
    unsigned char result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "dN"(port));
    return result;
}

static void outb(unsigned short port, unsigned char value) {
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static void handle_scancode(unsigned char scancode) {
    if (scancode & 0x80) {
        return;
    }
    if (scancode < 128 && keymap[scancode] != 0) {
        khukhuri_shell_handle_char(keymap[scancode]);
    }
}

void keyboard_init() {
    unsigned char mask = inb(0x21);
    mask &= (unsigned char)~0x02;
    outb(0x21, mask);
}

void keyboard_handler() {
    unsigned char scancode = inb(0x60);
    handle_scancode(scancode);
}

void keyboard_poll() {
    unsigned char status = inb(0x64);
    if (status & 0x01) {
        unsigned char scancode = inb(0x60);
        handle_scancode(scancode);
    }
}