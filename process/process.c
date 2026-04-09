#include "process.h"

static PCB process_table[MAX_PROCESSES];
static int process_count = 0;
static int next_pid = 1;

void create_process(void (*func)(void)) {
    if (process_count >= MAX_PROCESSES || func == 0) {
        return;
    }

    process_table[process_count].pid = next_pid++;
    process_table[process_count].state = PROCESS_READY;
    process_table[process_count].entry_point = func;
    process_count++;
}

void schedule(void) {
    /*
     * Milestone 1 stub:
     * No scheduling logic yet; this keeps module integration-ready.
     */
}
