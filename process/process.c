#include "process.h"

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
