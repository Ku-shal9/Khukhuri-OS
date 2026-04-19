#include "process.h"
#include "printf.h"

static PCB process_table[MAX_PROCESSES];
static int process_count = 0;
static int next_pid = 1;
static int current_index = -1;

void create_process(void (*func)(void)) {
    if (process_count >= MAX_PROCESSES || func == 0) {
        return;
    }

    process_table[process_count].pid = next_pid++;
    process_table[process_count].state = PROCESS_READY;
    process_table[process_count].priority = 1;
    process_table[process_count].entry_point = func;
    process_count++;
}

void schedule(void) {
    if (process_count == 0) {
        return;
    }

    if (current_index >= 0 && current_index < process_count) {
        if (process_table[current_index].state == PROCESS_RUNNING) {
            process_table[current_index].state = PROCESS_READY;
        }
    }

    int scanned = 0;
    int index = current_index;
    while (scanned < process_count) {
        index = (index + 1) % process_count;
        if (process_table[index].state == PROCESS_READY) {
            process_table[index].state = PROCESS_RUNNING;
            current_index = index;
            return;
        }
        scanned++;
    }
}

int get_process_count(void) {
    return process_count;
}

static const char* state_name(int s) {
    switch (s) {
        case PROCESS_UNUSED:
            return "UNUSED";
        case PROCESS_READY:
            return "READY";
        case PROCESS_RUNNING:
            return "RUNNING";
        case PROCESS_WAITING:
            return "WAITING";
        default:
            return "?";
    }
}

void process_dump_all(void) {
    printf("\033[1;36mPID  State     Prio  Entry\033[0m\n");
    if (process_count == 0) {
        printf("\033[1;33m(no processes)\033[0m\n");
        return;
    }
    for (int i = 0; i < process_count; i++) {
        printf(" %2d  \033[1;37m%s\033[0m %4d  %p\n",
            process_table[i].pid,
            state_name(process_table[i].state),
            process_table[i].priority,
            (void*)process_table[i].entry_point);
    }
    if (current_index >= 0 && current_index < process_count) {
        printf("\033[1;32mCurrent slot index: %d\033[0m\n", current_index);
    }
}

void process_schedule_times(int times) {
    for (int i = 0; i < times; i++) {
        schedule();
    }
    printf("\033[1;35mAfter %d schedule() step(s):\033[0m\n", times);
    process_dump_all();
}
