#include "keyboard.h"
#include "console.h"

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

static const char keymap_shift[128] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*',
    '(', ')', '_', '+', '\b', '\t', 'Q', 'W', 'E', 'R',
    'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0,
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',
    '"', '~', 0,  '|', 'Z', 'X', 'C', 'V', 'B', 'N',
    'M', '<', '>', '?', 0,  '*', 0,  ' ', 0,  0,
    0,  0,   0,   0,   0,   0,   0,  0,   0,  0,
    0,  0,   0,   0,   '-', 0,   0,  0,   '+', 0,
    0,  0,   0,   0,   0,   0,   0,  0,   0,  0,
    0,  0,   0,   0,   0,   0,   0,  0,   0,  0,
    0,  0,   0,   0,   0,   0,   0,  0,   0,  0,
    0,  0,   0,   0,   0,   0,   0,  0
};

static int extended_prefix = 0;
static int shift_pressed = 0;

static unsigned char inb(unsigned short port) {
    unsigned char result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "dN"(port));
    return result;
}

static void outb(unsigned short port, unsigned char value) {
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static void handle_scancode(unsigned char scancode) {
    if (scancode == 0xE0) {
        extended_prefix = 1;
        return;
    }

    if (scancode == 0x2A || scancode == 0x36) {
        shift_pressed = 1;
        return;
    }
    if (scancode == 0xAA || scancode == 0xB6) {
        shift_pressed = 0;
        return;
    }

    if (extended_prefix) {
        extended_prefix = 0;
        if (scancode & 0x80) {
            return;
        }
        console_on_extended_scancode(scancode);
        return;
    }

    if (scancode & 0x80) {
        return;
    }

    if (scancode < 128) {
        char ch = shift_pressed ? keymap_shift[scancode] : keymap[scancode];
        if (ch != 0) {
            console_on_key(ch);
        }
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
