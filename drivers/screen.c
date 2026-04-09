#include "screen.h"

#define VIDEO_MEMORY 0xB8000
#define MAX_ROWS 25
#define MAX_COLS 80

static int cursor_pos = 0;

void clear_screen(void) {
    char* vidmem = (char*) VIDEO_MEMORY;

    for (int i = 0; i < MAX_ROWS * MAX_COLS * 2; i += 2) {
        vidmem[i] = ' ';
        vidmem[i + 1] = 0x07;
    }

    cursor_pos = 0;
}

void print(const char* str) {
    char* vidmem = (char*) VIDEO_MEMORY;

    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == '\n') {
            int row = cursor_pos / (MAX_COLS * 2);
            cursor_pos = (row + 1) * MAX_COLS * 2;
            continue;
        }

        if (cursor_pos >= MAX_ROWS * MAX_COLS * 2) {
            cursor_pos = 0;
        }

        vidmem[cursor_pos++] = str[i];
        vidmem[cursor_pos++] = 0x07;
    }
}