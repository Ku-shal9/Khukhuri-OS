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

static const char keymap_set2[128] = {
    [0x0D] = '\t',
    [0x0E] = '`',
    [0x15] = 'q', [0x16] = '1', [0x1A] = 'z', [0x1B] = 's', [0x1C] = 'a', [0x1D] = 'w', [0x1E] = '2',
    [0x21] = 'c', [0x22] = 'x', [0x23] = 'd', [0x24] = 'e', [0x25] = '4', [0x26] = '3',
    [0x29] = ' ',
    [0x2A] = 'v', [0x2B] = 'f', [0x2C] = 't', [0x2D] = 'r', [0x2E] = '5',
    [0x31] = 'n', [0x32] = 'b', [0x33] = 'h', [0x34] = 'g', [0x35] = 'y', [0x36] = '6',
    [0x3A] = 'm', [0x3B] = 'j', [0x3C] = 'u', [0x3D] = '7', [0x3E] = '8',
    [0x41] = ',', [0x42] = 'k', [0x43] = 'i', [0x44] = 'o', [0x45] = '0', [0x46] = '9',
    [0x49] = '.', [0x4A] = '/', [0x4B] = 'l', [0x4C] = ';', [0x4D] = 'p', [0x4E] = '-',
    [0x52] = '\'', [0x54] = '[', [0x55] = '=',
    [0x5A] = '\n', [0x5B] = ']', [0x5D] = '\\',
    [0x66] = '\b'
};

static int set2_break_pending = 0;
static int set2_extended_pending = 0;

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
        set2_extended_pending = 1;
        return;
    }
    if (scancode == 0xF0) {
        set2_break_pending = 1;
        return;
    }

    if (!set2_break_pending && scancode < 128 && keymap_set2[scancode] != 0) {
        khukhuri_shell_handle_char(keymap_set2[scancode]);
        set2_extended_pending = 0;
        return;
    }

    if ((scancode & 0x80) == 0 && scancode < 128 && keymap[scancode] != 0) {
        khukhuri_shell_handle_char(keymap[scancode]);
    }

    if (set2_break_pending) {
        set2_break_pending = 0;
    }
    if (set2_extended_pending) {
        set2_extended_pending = 0;
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