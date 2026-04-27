#include "console.h"
#include "screen.h"
#include "printf.h"
#include "khukhuri_shell.h"

#define BOOT_PIN "1111"
#define MAX_PIN_FAILS 5
#define GATE_LINE_LEN 64

typedef enum {
    CON_PIN,
    CON_GATE,
    CON_SHELL,
    CON_LOCKOUT
} ConsoleState;

static ConsoleState state = CON_PIN;
static char line_buf[GATE_LINE_LEN];
static int line_pos = 0;
static int pin_fails = 0;

static int kstrcmp(const char* a, const char* b) {
    int i = 0;
    while (a[i] != '\0' && b[i] != '\0') {
        if (a[i] != b[i]) {
            return (int)((unsigned char)a[i] - (unsigned char)b[i]);
        }
        i++;
    }
    return (int)((unsigned char)a[i] - (unsigned char)b[i]);
}

static void pin_prompt(void) {
    printf("\033[1;33mPIN>\033[0m ");
}

static void gate_prompt(void) {
    printf("\033[1;36m>\033[0m ");
}

static void handle_pin_line(void) {
    line_buf[line_pos] = '\0';
    if (kstrcmp(line_buf, BOOT_PIN) == 0) {
        printf("\033[1;32mAccess granted.\033[0m\n");
        printf("Type \033[1m%s\033[0m and press Enter to open the shell.\n", "shell");
        state = CON_GATE;
        gate_prompt();
    } else {
        pin_fails++;
        printf("\033[1;31mInvalid PIN.\033[0m (%d/%d)\n", pin_fails, MAX_PIN_FAILS);
        if (pin_fails >= MAX_PIN_FAILS) {
            printf("\033[1;31mLocked. Reset VM to try again.\033[0m\n");
            state = CON_LOCKOUT;
            while (1) {
                __asm__ volatile("hlt");
            }
        }
        pin_prompt();
    }
    line_pos = 0;
}

static void handle_gate_line(void) {
    line_buf[line_pos] = '\0';
    if (kstrcmp(line_buf, "shell") == 0) {
        state = CON_SHELL;
        printf("\033[1;32mStarting LemonJuice shell...\033[0m\n\n");
        khukhuri_shell_init();
    } else {
        printf("Type exactly: shell\n");
        gate_prompt();
    }
    line_pos = 0;
}

void console_init(void) {
    state = CON_PIN;
    line_pos = 0;
    pin_fails = 0;
    printf("\033[1;35mSecurity\033[0m: enter boot PIN (set in console.c as BOOT_PIN), then type shell.\n");
    pin_prompt();
}

void console_on_key(char c) {
    if (state == CON_LOCKOUT) {
        return;
    }
    if (state == CON_SHELL) {
        khukhuri_shell_handle_char(c);
        return;
    }

    if (c == '\n') {
        putchar('\n');
        if (state == CON_PIN) {
            handle_pin_line();
        } else if (state == CON_GATE) {
            handle_gate_line();
        }
        return;
    }

    if (c == '\b') {
        if (line_pos > 0) {
            line_pos--;
            putchar('\b');
        }
        return;
    }

    if (line_pos < GATE_LINE_LEN - 1) {
        line_buf[line_pos++] = c;
        if (state == CON_PIN) {
            putchar('*');
        } else {
            putchar(c);
        }
    }
}

void console_leave_shell(void) {
    if (state != CON_SHELL) {
        return;
    }
    state = CON_GATE;
    printf("\033[1;33mShell closed.\033[0m Type \033[1mshell\033[0m again to return.\n");
    gate_prompt();
}

void console_on_extended_scancode(unsigned char code) {
    if (state == CON_LOCKOUT) {
        return;
    }
    if (code & 0x80) {
        return;
    }
    if (code == 0x48) {
        screen_scroll_view(-1);
    } else if (code == 0x50) {
        screen_scroll_view(1);
    }
}
