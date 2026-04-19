#ifndef CONSOLE_H
#define CONSOLE_H

void console_init(void);
void console_on_key(char c);
void console_on_extended_scancode(unsigned char code);

#endif
