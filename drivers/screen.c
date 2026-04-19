#include "screen.h"
#include <stdint.h>

#define VIDEO_MEMORY 0xB8000
#define MAX_ROWS 25
#define MAX_COLS 80
#define MAX_LOG 200

static uint16_t lines[MAX_LOG][MAX_COLS];
static int cur_row = 0;
static int cur_col = 0;
static int scroll_off = 0;
static unsigned char current_attr = 0x07;
static unsigned char ansi_bold = 0;

static int esc_state = 0;
static char esc_buf[32];
static int esc_len = 0;

static int mouse_cell_x = 40;
static int mouse_cell_y = 12;
static int mouse_visible = 0;

static uint16_t make_cell(char c) {
    return (uint16_t)(((unsigned short)current_attr << 8) | (unsigned char)c);
}

static void ensure_row_blank(int r) {
    for (int x = 0; x < MAX_COLS; x++) {
        lines[r][x] = make_cell(' ');
    }
}

static void apply_sgr_token(int code) {
    if (code == 0) {
        current_attr = 0x07;
        ansi_bold = 0;
        return;
    }
    if (code == 1) {
        ansi_bold = 1;
        return;
    }
    if (code == 22) {
        ansi_bold = 0;
        return;
    }
    if (code == 39) {
        current_attr = (unsigned char)((current_attr & 0xF0) | 0x07);
        return;
    }
    if (code >= 30 && code <= 37) {
        unsigned char fg = (unsigned char)(code - 30);
        if (ansi_bold) {
            fg = (unsigned char)(fg + 8);
        }
        current_attr = (unsigned char)((current_attr & 0xF0) | (fg & 0x0F));
        return;
    }
    if (code >= 90 && code <= 97) {
        unsigned char fg = (unsigned char)(8 + (code - 90));
        current_attr = (unsigned char)((current_attr & 0xF0) | (fg & 0x0F));
        return;
    }
}

static void flush_esc_sequence(void) {
    esc_buf[esc_len] = '\0';
    int i = 0;
    while (i < esc_len) {
        int start = i;
        while (i < esc_len && esc_buf[i] != ';') {
            i++;
        }
        int n = 0;
        for (int j = start; j < i; j++) {
            if (esc_buf[j] >= '0' && esc_buf[j] <= '9') {
                n = n * 10 + (esc_buf[j] - '0');
            }
        }
        apply_sgr_token(n);
        if (i < esc_len && esc_buf[i] == ';') {
            i++;
        }
    }
    esc_len = 0;
    esc_state = 0;
}

static void screen_redraw_hw(void) {
    uint16_t* vm = (uint16_t*)VIDEO_MEMORY;
    for (int r = 0; r < MAX_ROWS; r++) {
        int lr = scroll_off + r;
        if (lr >= 0 && lr < MAX_LOG) {
            for (int c = 0; c < MAX_COLS; c++) {
                vm[r * MAX_COLS + c] = lines[lr][c];
            }
        } else {
            for (int c = 0; c < MAX_COLS; c++) {
                vm[r * MAX_COLS + c] = make_cell(' ');
            }
        }
    }

    if (mouse_visible && mouse_cell_y >= 0 && mouse_cell_y < MAX_ROWS
        && mouse_cell_x >= 0 && mouse_cell_x < MAX_COLS) {
        int idx = mouse_cell_y * MAX_COLS + mouse_cell_x;
        uint16_t cur = vm[idx];
        unsigned char ch = (unsigned char)(cur & 0xFF);
        unsigned char at = (unsigned char)((cur >> 8) & 0xFF);
        at = (unsigned char)((at & 0xF0) | ((at & 0x0F) ^ 0x0F));
        vm[idx] = (uint16_t)(((unsigned short)at << 8) | ch);
    }
}

static void clamp_scroll(void) {
    int max_scroll = cur_row - (MAX_ROWS - 1);
    if (max_scroll < 0) {
        max_scroll = 0;
    }
    if (scroll_off < 0) {
        scroll_off = 0;
    }
    if (scroll_off > max_scroll) {
        scroll_off = max_scroll;
    }
}

static void follow_cursor(void) {
    if (cur_row >= scroll_off + MAX_ROWS) {
        scroll_off = cur_row - (MAX_ROWS - 1);
    }
    clamp_scroll();
}

static void advance_row(void) {
    cur_col = 0;
    cur_row++;
    if (cur_row >= MAX_LOG) {
        for (int r = 1; r < MAX_LOG; r++) {
            for (int c = 0; c < MAX_COLS; c++) {
                lines[r - 1][c] = lines[r][c];
            }
        }
        cur_row = MAX_LOG - 1;
        ensure_row_blank(cur_row);
    } else {
        ensure_row_blank(cur_row);
    }
    follow_cursor();
}

void clear_screen(void) {
    current_attr = 0x07;
    ansi_bold = 0;
    esc_state = 0;
    esc_len = 0;
    cur_row = 0;
    cur_col = 0;
    scroll_off = 0;
    for (int r = 0; r < MAX_LOG; r++) {
        ensure_row_blank(r);
    }
    screen_redraw_hw();
}

void screen_scroll_view(int delta_lines) {
    scroll_off += delta_lines;
    clamp_scroll();
    screen_redraw_hw();
}

void screen_mouse_wheel(int delta) {
    screen_scroll_view(delta);
}

void screen_mouse_set_cell(int x, int y) {
    if (x < 0) {
        x = 0;
    }
    if (x >= MAX_COLS) {
        x = MAX_COLS - 1;
    }
    if (y < 0) {
        y = 0;
    }
    if (y >= MAX_ROWS) {
        y = MAX_ROWS - 1;
    }
    mouse_cell_x = x;
    mouse_cell_y = y;
    mouse_visible = 1;
    screen_redraw_hw();
}

void putchar(char c) {
    if (esc_state == 0 && c == '\033') {
        esc_state = 1;
        esc_len = 0;
        return;
    }
    if (esc_state == 1) {
        if (c == '[') {
            esc_state = 2;
        } else {
            esc_state = 0;
        }
        return;
    }
    if (esc_state == 2) {
        if ((c >= '0' && c <= '9') || c == ';') {
            if (esc_len < (int)sizeof(esc_buf) - 1) {
                esc_buf[esc_len++] = c;
            }
            return;
        }
        if (c == 'm') {
            flush_esc_sequence();
            screen_redraw_hw();
            return;
        }
        esc_state = 0;
        esc_len = 0;
        return;
    }

    if (c == '\n') {
        advance_row();
        screen_redraw_hw();
        return;
    }
    if (c == '\b') {
        if (cur_col > 0) {
            cur_col--;
            lines[cur_row][cur_col] = make_cell(' ');
        } else if (cur_row > 0) {
            cur_row--;
            cur_col = MAX_COLS - 1;
            lines[cur_row][cur_col] = make_cell(' ');
        }
        screen_redraw_hw();
        return;
    }
    if (c == '\r') {
        cur_col = 0;
        screen_redraw_hw();
        return;
    }

    lines[cur_row][cur_col] = make_cell(c);
    cur_col++;
    if (cur_col >= MAX_COLS) {
        advance_row();
    }
    follow_cursor();
    screen_redraw_hw();
}

void print(const char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        putchar(str[i]);
    }
}
