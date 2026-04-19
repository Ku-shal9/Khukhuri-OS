#include "khukhuri_shell.h"
#include "screen.h"
#include "printf.h"
#include "timer.h"
#include "filesystem.h"
#include "fat.h"
#include "memory.h"
#include "process.h"
#include "console.h"

#define KSHELL_MAX_LINE 256
#define KSHELL_MAX_TOKENS 16

static char line_buffer[KSHELL_MAX_LINE];
static int line_pos = 0;
static void* alloc_slots[8];

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
    print("\033[33mlemon\033[0m\033[1;34m@khukhuri>\033[0m ");
}

static void cmd_help(void) {
    printf("\033[1;36mMiniShell by Kushal — Kernel Mode\033[0m\n");
    printf("  \033[1;32msahayog\033[0m              Show this help\n");
    printf("  \033[1;32mparibartan\033[0m [dir]   Change directory (stub)\n");
    printf("  \033[1;32mnikli\033[0m                Exit shell (return to gate; type shell again)\n");
    printf("  \033[1;32mjhalak\033[0m               FAT + file listing summary\n");
    printf("  \033[1;32mfatlist\033[0m              List all files (name, size, start block)\n");
    printf("  \033[1;32msamaya\033[0m               Timer ticks\n");
    printf("  \033[1;32mshusankhya\033[0m a op b   Arithmetic (+ - * /)\n");
    printf("  \033[1;32mcreate\033[0m / write / read  File operations\n");
    printf("  \033[1;32mmemtest\033[0m              Allocator stress (malloc/free)\n");
    printf("  \033[1;32mproctest\033[0m             Create dummy processes + 2 schedule() steps\n");
    printf("  \033[1;32mproclist\033[0m             Process table + current policy\n");
    printf("  \033[1;32msched\033[0m [n]            Run schedule() n times (default 5) + stats + table\n");
    printf("  \033[1;32mpolicy\033[0m [rr|fcfs|hpf]  Set scheduler (HPF = highest priority first; alias: hfin)\n");
    printf("  \033[1;32mstats\033[0m                schedule() calls & context switches\n");
    printf("  \033[1;32mbench\033[0m                Run 9 steps x3 policies (RR, FCFS, HPF) with priorities 1,2,3\n");
    printf("  \033[1;32mmkproc\033[0m [prio]        Create one dummy PCB (default prio 5)\n");
    printf("  \033[1;33mPgUp/PgDn\033[0m scroll the boot log when the screen fills.\n");
}

static void cmd_calc(char* argv[], int argc) {
    if (argc < 4) {
        printf("\033[1;31mFormat:\033[0m shusankhya <num1> <op> <num2>\n");
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
            printf("\033[1;31mMath Error\033[0m\n");
            return;
        }
        result = x / y;
    } else {
        printf("\033[1;31mUnknown operator\033[0m (use + - * /)\n");
        return;
    }

    printf("\033[1;36mResult:\033[0m ");
    print_int(result);
    putchar('\n');
}

static void cmd_write(char* argv[], int argc) {
    if (argc < 3) {
        printf("\033[1;31mFormat:\033[0m write <name> <text>\n");
        return;
    }
    if (write_file(argv[1], argv[2], kstrlen(argv[2])) == 0) {
        printf("\033[1;32mWrite success\033[0m\n");
    } else {
        printf("\033[1;31mWrite failed\033[0m\n");
    }
}

static void dummy_process(void) {
}

static void cmd_memtest(void) {
    int i;
    printf("\033[1;35mmemtest:\033[0m allocating 8 blocks, freeing even slots…\n");
    for (i = 0; i < 8; i++) {
        alloc_slots[i] = os_malloc(32 + (i * 8));
    }
    for (i = 0; i < 8; i += 2) {
        if (alloc_slots[i] != 0) {
            os_free(alloc_slots[i]);
            alloc_slots[i] = 0;
        }
    }
    printf("\033[1;32mDone.\033[0m Exercises \033[1mos_malloc\033[0m / \033[1mos_free\033[0m and block split/coalesce.\n");
}

static void cmd_proctest(void) {
    int before = get_process_count();
    create_process_prio(dummy_process, 1);
    create_process_prio(dummy_process, 2);
    schedule();
    schedule();
    printf("\033[1;35mproctest:\033[0m added 2 PCBs (prio 1 and 2); ran schedule() twice.\n");
    printf("  process count before: ");
    print_int(before);
    printf("  after: ");
    print_int(get_process_count());
    putchar('\n');
    printf("  Try \033[1;36mpolicy hpf\033[0m then \033[1;36mproclist\033[0m / \033[1;36msched 5\033[0m.\n");
}

static void cmd_policy(char** argv, int argc) {
    if (argc < 2) {
        printf("\033[1;36mCurrent policy:\033[0m \033[1;37m%s\033[0m\n", process_policy_name(process_get_policy()));
        printf("\033[1;33mUsage:\033[0m policy rr | fcfs | hpf  (hpf = highest priority first; alias \033[1mhfin\033[0m)\n");
        return;
    }
    if (kstrcmp(argv[1], "rr") == 0) {
        process_set_policy(SCHED_RR);
    } else if (kstrcmp(argv[1], "fcfs") == 0) {
        process_set_policy(SCHED_FCFS);
    } else if (kstrcmp(argv[1], "hpf") == 0 || kstrcmp(argv[1], "hfin") == 0) {
        process_set_policy(SCHED_HPF);
    } else {
        printf("\033[1;31mUnknown policy.\033[0m Use rr, fcfs, or hpf.\n");
        return;
    }
    printf("\033[1;32mPolicy set to %s\033[0m\n", process_policy_name(process_get_policy()));
}

static void cmd_stats(void) {
    sched_stats_print();
}

static void cmd_bench(void) {
    SchedPolicy pols[] = { SCHED_RR, SCHED_FCFS, SCHED_HPF };
    const char* titles[] = {
        "RR (round robin, circular)",
        "FCFS (FIFO by creation index)",
        "HPF (highest priority wins; tie: lower PID)"
    };
    printf("\033[1;35mBenchmark:\033[0m 3 tasks prio 1,2,3 — 9 schedule() steps each.\n\n");
    for (int p = 0; p < 3; p++) {
        process_reset_all();
        sched_stats_reset();
        process_set_policy(pols[p]);
        create_process_prio(dummy_process, 1);
        create_process_prio(dummy_process, 2);
        create_process_prio(dummy_process, 3);
        printf("\033[1;33m--- %s ---\033[0m\n", titles[p]);
        for (int i = 0; i < 9; i++) {
            schedule();
        }
        sched_stats_print();
        process_dump_all();
        printf("\n");
    }
    printf("\033[1;32mCompare context switch counts across policies.\033[0m\n");
}

static void cmd_mkproc(char** argv, int argc) {
    int prio = 5;
    if (argc >= 2) {
        prio = katoi(argv[1]);
    }
    if (prio < 0) {
        prio = 0;
    }
    create_process_prio(dummy_process, prio);
    printf("\033[1;32mCreated\033[0m dummy PCB with priority ");
    print_int(prio);
    putchar('\n');
}

static void cmd_jhalak(void) {
    printf("\033[1;36m— In-memory FAT filesystem —\033[0m\n");
    printf("  Blocks: %d  Block size: %d  Free blocks: \033[1;32m%d\033[0m\n",
        MAX_BLOCKS, BLOCK_SIZE, fat_count_free());
    fs_list_files();
}

static int execute_line(char* line) {
    char* argv[KSHELL_MAX_TOKENS];
    int argc = tokenize(line, argv);
    if (argc == 0) {
        return 1;
    }

    if (kstrcmp(argv[0], "sahayog") == 0) {
        cmd_help();
    } else if (kstrcmp(argv[0], "paribartan") == 0) {
        printf("\033[1;33mparibartan:\033[0m not available in kernel mode (no real paths).\n");
    } else if (kstrcmp(argv[0], "nikli") == 0) {
        printf("\033[1;33mGoodbye — returning to secure gate.\033[0m\n");
        console_leave_shell();
        return 0;
    } else if (kstrcmp(argv[0], "jhalak") == 0) {
        cmd_jhalak();
    } else if (kstrcmp(argv[0], "fatlist") == 0) {
        fs_list_files();
    } else if (kstrcmp(argv[0], "samaya") == 0) {
        printf("\033[1;36mticks:\033[0m ");
        print_int((int)timer_ticks());
        putchar('\n');
    } else if (kstrcmp(argv[0], "shusankhya") == 0) {
        cmd_calc(argv, argc);
    } else if (kstrcmp(argv[0], "create") == 0) {
        if (argc < 2) {
            printf("\033[1;31mFormat:\033[0m create <name>\n");
        } else if (create_file(argv[1]) == 0) {
            printf("\033[1;32mFile created\033[0m\n");
        } else {
            printf("\033[1;31mCreate failed\033[0m\n");
        }
    } else if (kstrcmp(argv[0], "write") == 0) {
        cmd_write(argv, argc);
    } else if (kstrcmp(argv[0], "read") == 0) {
        char read_buf[1024];
        int n;
        if (argc < 2) {
            printf("\033[1;31mFormat:\033[0m read <name>\n");
        } else {
            n = read_file(argv[1], read_buf);
            if (n <= 0) {
                printf("\033[1;31mRead failed\033[0m\n");
            } else {
                read_buf[n] = '\0';
                printf("\033[1;37m");
                print(read_buf);
                printf("\033[0m\n");
            }
        }
    } else if (kstrcmp(argv[0], "memtest") == 0) {
        cmd_memtest();
    } else if (kstrcmp(argv[0], "proctest") == 0) {
        cmd_proctest();
    } else if (kstrcmp(argv[0], "proclist") == 0) {
        process_dump_all();
    } else if (kstrcmp(argv[0], "sched") == 0) {
        int n = 5;
        if (argc >= 2) {
            n = katoi(argv[1]);
        }
        if (n < 1) {
            n = 1;
        }
        if (n > 64) {
            n = 64;
        }
        process_schedule_times(n);
    } else if (kstrcmp(argv[0], "policy") == 0) {
        cmd_policy(argv, argc);
    } else if (kstrcmp(argv[0], "stats") == 0) {
        cmd_stats();
    } else if (kstrcmp(argv[0], "bench") == 0) {
        cmd_bench();
    } else if (kstrcmp(argv[0], "mkproc") == 0) {
        cmd_mkproc(argv, argc);
    } else {
        printf("\033[1;31mUnknown:\033[0m ");
        print(argv[0]);
        putchar('\n');
    }
    return 1;
}

void khukhuri_shell_init(void) {
    line_pos = 0;
    printf("\033[1;32mLemonJuice\033[0m shell (kernel). Type \033[1;36msahayog\033[0m or \033[1;36mnikli\033[0m to exit.\n");
    print_prompt();
}

void khukhuri_shell_handle_char(char c) {
    if (c == '\n') {
        putchar('\n');
        line_buffer[line_pos] = '\0';
        if (!execute_line(line_buffer)) {
            line_pos = 0;
            line_buffer[0] = '\0';
            return;
        }
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
