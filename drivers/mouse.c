#include "mouse.h"
#include "screen.h"

static void outb(unsigned short port, unsigned char value) {
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static unsigned char inb(unsigned short port) {
    unsigned char v;
    __asm__ volatile("inb %1, %0" : "=a"(v) : "Nd"(port));
    return v;
}

static void mouse_wait(unsigned char type) {
    unsigned int timeout = 100000;
    if (type == 0) {
        while (timeout--) {
            if ((inb(0x64) & 1) == 1) {
                return;
            }
        }
    } else {
        while (timeout--) {
            if ((inb(0x64) & 2) == 0) {
                return;
            }
        }
    }
}

static void mouse_write(unsigned char value) {
    mouse_wait(1);
    outb(0xD4, 0x64);
    mouse_wait(1);
    outb(value, 0x60);
}

static void mouse_write_cmd(unsigned char value) {
    mouse_write(value);
    mouse_wait(0);
    (void)inb(0x60);
}

static int mouse_cycle = 0;
static unsigned char mouse_packet[3];
static int acc_dx = 0;
static int acc_dy = 0;
static int mouse_cell_x = 40;
static int mouse_cell_y = 12;

void mouse_init(void) {
    unsigned char mask;

    mouse_wait(1);
    outb(0xA8, 0x64);

    mouse_wait(1);
    outb(0x20, 0x64);
    mouse_wait(0);
    mask = (unsigned char)(inb(0x60) | 2);
    mouse_wait(1);
    outb(0x60, 0x64);
    mouse_wait(1);
    outb(mask, 0x60);

    mouse_write_cmd(0xF4);

    mask = inb(0xA1);
    mask &= (unsigned char)~0x10;
    outb(0xA1, mask);

    mouse_cycle = 0;
    acc_dx = 0;
    acc_dy = 0;
    screen_mouse_set_cell(mouse_cell_x, mouse_cell_y);
}

void mouse_handler(void) {
    unsigned char b = inb(0x60);

    if (mouse_cycle == 0 && (b & 0x08) == 0) {
        return;
    }

    mouse_packet[mouse_cycle++] = b;
    if (mouse_cycle < 3) {
        return;
    }
    mouse_cycle = 0;

    signed char dx = (signed char)mouse_packet[1];
    signed char dy = (signed char)mouse_packet[2];

    acc_dx += (int)dx;
    acc_dy += (int)dy;

    while (acc_dx >= 8) {
        acc_dx -= 8;
        mouse_cell_x++;
    }
    while (acc_dx <= -8) {
        acc_dx += 8;
        mouse_cell_x--;
    }
    while (acc_dy >= 8) {
        acc_dy -= 8;
        mouse_cell_y++;
    }
    while (acc_dy <= -8) {
        acc_dy += 8;
        mouse_cell_y--;
    }

    screen_mouse_set_cell(mouse_cell_x, mouse_cell_y);
}
