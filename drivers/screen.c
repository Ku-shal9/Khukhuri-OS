#include "screen.h"

#define VIDEO_MEMORY 0xB8000
#define MAX_ROWS 25
#define MAX_COLS 80

static int cursor_pos = 0;
static const unsigned char default_color = 0x07;

static void scroll_if_needed(void) {
    char* vidmem = (char*)VIDEO_MEMORY;
    int screen_size = MAX_ROWS * MAX_COLS * 2;
    if (cursor_pos < screen_size) {
        return;
    }

    for (int i = 0; i < (MAX_ROWS - 1) * MAX_COLS * 2; i++) {
        vidmem[i] = vidmem[i + MAX_COLS * 2];
    }
    for (int i = (MAX_ROWS - 1) * MAX_COLS * 2; i < screen_size; i += 2) {
        vidmem[i] = ' ';
        vidmem[i + 1] = default_color;
    }
    cursor_pos = (MAX_ROWS - 1) * MAX_COLS * 2;
}

void clear_screen(void) {
    char* vidmem = (char*) VIDEO_MEMORY;

    for (int i = 0; i < MAX_ROWS * MAX_COLS * 2; i += 2) {
        vidmem[i] = ' ';
        vidmem[i + 1] = default_color;
    }

    cursor_pos = 0;
}

void putchar(char c) {
    char* vidmem = (char*) VIDEO_MEMORY;

    if (c == '\n') {
        int row = cursor_pos / (MAX_COLS * 2);
        cursor_pos = (row + 1) * MAX_COLS * 2;
        scroll_if_needed();
        return;
    }
    if (c == '\b') {
        if (cursor_pos > 0) {
            cursor_pos -= 2;
            vidmem[cursor_pos] = ' ';
            vidmem[cursor_pos + 1] = default_color;
        }
        return;
    }

    vidmem[cursor_pos++] = c;
    vidmem[cursor_pos++] = default_color;
    scroll_if_needed();
}

void print(const char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        putchar(str[i]);
    }
}