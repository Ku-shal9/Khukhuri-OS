#include "khukhuri_shell.h"
#include "screen.h"
#include "timer.h"
#include "filesystem.h"

#define KSHELL_MAX_LINE 256
#define KSHELL_MAX_TOKENS 16

static char line_buffer[KSHELL_MAX_LINE];
static int line_pos = 0;

static int kstrlen(const char* s) {
    int n = 0;
    while (s[n] != '\0') {
        n++;
    }
    return n;
}

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

static int katoi(const char* s) {
    int sign = 1;
    int value = 0;
    int i = 0;
    if (s[0] == '-') {
        sign = -1;
        i = 1;
    }
    while (s[i] >= '0' && s[i] <= '9') {
        value = value * 10 + (s[i] - '0');
        i++;
    }
    return value * sign;
}

static void print_int(int value) {
    char out[16];
    int idx = 0;
    if (value == 0) {
        putchar('0');
        return;
    }
    if (value < 0) {
        putchar('-');
        value = -value;
    }
    while (value > 0 && idx < 15) {
        out[idx++] = (char)('0' + (value % 10));
        value /= 10;
    }
    while (idx > 0) {
        putchar(out[--idx]);
    }
}

static int tokenize(char* line, char* argv[]) {
    int argc = 0;
    int i = 0;
    while (line[i] != '\0' && argc < KSHELL_MAX_TOKENS) {
        while (line[i] == ' ') {
            i++;
        }
        if (line[i] == '\0') {
            break;
        }
        argv[argc++] = &line[i];
        while (line[i] != '\0' && line[i] != ' ') {
            i++;
        }
        if (line[i] == '\0') {
            break;
        }
        line[i] = '\0';
        i++;
    }
    return argc;
}

static void print_prompt(void) {
    print("lemon@khukhuri> ");
}

static void cmd_help(void) {
    print("MiniShell by Kushal - Kernel Mode\n");
    print("  sahayog                Show help\n");
    print("  paribartan [dir]       Change directory (stub)\n");
    print("  nikli                  Exit shell loop (stub)\n");
    print("  jhalak                 Show basic FS info\n");
    print("  samaya                 Show timer ticks\n");
    print("  shusankhya a op b      Simple arithmetic\n");
    print("  create <name>          Create file\n");
    print("  write <name> <text>    Write text to file\n");
    print("  read <name>            Read file\n");
}

static void cmd_calc(char* argv[], int argc) {
    if (argc < 4) {
        print("Format: shusankhya <num1> <operator> <num2>\n");
        return;
    }

    int x = katoi(argv[1]);
    int y = katoi(argv[3]);
    char op = argv[2][0];
    int result = 0;

    if (op == '+') {
        result = x + y;
    } else if (op == '-') {
        result = x - y;
    } else if (op == '*') {
        result = x * y;
    } else if (op == '/') {
        if (y == 0) {
            print("Math Error\n");
            return;
        }
        result = x / y;
    } else {
        print("Unknown Operator, try + - * /\n");
        return;
    }

    print("Result: ");
    print_int(result);
    putchar('\n');
}

static void cmd_write(char* argv[], int argc) {
    if (argc < 3) {
        print("Format: write <name> <text>\n");
        return;
    }
    if (write_file(argv[1], argv[2], kstrlen(argv[2])) == 0) {
        print("Write success\n");
    } else {
        print("Write failed\n");
    }
}

static void execute_line(char* line) {
    char* argv[KSHELL_MAX_TOKENS];
    int argc = tokenize(line, argv);
    if (argc == 0) {
        return;
    }

    if (kstrcmp(argv[0], "sahayog") == 0) {
        cmd_help();
    } else if (kstrcmp(argv[0], "paribartan") == 0) {
        print("paribartan is not available in kernel mode\n");
    } else if (kstrcmp(argv[0], "nikli") == 0) {
        print("nikli requested - shell remains active in kernel mode\n");
    } else if (kstrcmp(argv[0], "jhalak") == 0) {
        print("In-memory filesystem ready\n");
    } else if (kstrcmp(argv[0], "samaya") == 0) {
        print("ticks: ");
        print_int((int)timer_ticks());
        putchar('\n');
    } else if (kstrcmp(argv[0], "shusankhya") == 0) {
        cmd_calc(argv, argc);
    } else if (kstrcmp(argv[0], "create") == 0) {
        if (argc < 2) {
            print("Format: create <name>\n");
        } else if (create_file(argv[1]) == 0) {
            print("File created\n");
        } else {
            print("Create failed\n");
        }
    } else if (kstrcmp(argv[0], "write") == 0) {
        cmd_write(argv, argc);
    } else if (kstrcmp(argv[0], "read") == 0) {
        char read_buf[1024];
        int n;
        if (argc < 2) {
            print("Format: read <name>\n");
        } else {
            n = read_file(argv[1], read_buf);
            if (n <= 0) {
                print("Read failed\n");
            } else {
                read_buf[n] = '\0';
                print(read_buf);
                putchar('\n');
            }
        }
    } else {
        print("Unknown command: ");
        print(argv[0]);
        putchar('\n');
    }
}

void khukhuri_shell_init(void) {
    line_pos = 0;
    print("LemonJuice shell embedded in Khukhuri-OS\n");
    print("Type 'sahayog' for commands\n");
    print_prompt();
}

void khukhuri_shell_handle_char(char c) {
    if (c == '\n') {
        putchar('\n');
        line_buffer[line_pos] = '\0';
        execute_line(line_buffer);
        line_pos = 0;
        line_buffer[0] = '\0';
        print_prompt();
        return;
    }

    if (c == '\b') {
        if (line_pos > 0) {
            line_pos--;
            line_buffer[line_pos] = '\0';
            putchar('\b');
        }
        return;
    }

    if (line_pos < KSHELL_MAX_LINE - 1) {
        line_buffer[line_pos++] = c;
        line_buffer[line_pos] = '\0';
        putchar(c);
    }
}
