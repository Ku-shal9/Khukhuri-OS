#ifndef SCREEN_H
#define SCREEN_H

void clear_screen(void);
void putchar(char c);
void print(const char* str);

void screen_scroll_view(int delta_lines);
void screen_mouse_set_cell(int x, int y);
void screen_mouse_wheel(int delta);

#endif
