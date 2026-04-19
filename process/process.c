#include "process.h"
#include "printf.h"

static PCB process_table[MAX_PROCESSES];
static int process_count = 0;
static int next_pid = 1;
static int current_index = -1;
static SchedPolicy sched_policy = SCHED_RR;

static unsigned long stat_schedule_calls = 0;
static unsigned long stat_context_switches = 0;

static void demote_running(void) {
    if (current_index >= 0 && current_index < process_count) {
        if (process_table[current_index].state == PROCESS_RUNNING) {
            process_table[current_index].state = PROCESS_READY;
        }
    }
}

static void pick_running(int new_index) {
    if (new_index < 0 || new_index >= process_count) {
        return;
    }
    if (process_table[new_index].state != PROCESS_READY) {
        return;
    }
    if (current_index != new_index) {
        stat_context_switches++;
    }
    process_table[new_index].state = PROCESS_RUNNING;
    current_index = new_index;
}

static void schedule_rr(void) {
    if (process_count == 0) {
        return;
    }
    int scanned = 0;
    int index = current_index;
    while (scanned < process_count) {
        index = (index + 1) % process_count;
        if (process_table[index].state == PROCESS_READY) {
            pick_running(index);
            return;
        }
        scanned++;
    }
}

static void schedule_fcfs(void) {
    if (process_count == 0) {
        return;
    }
    int start = (current_index < 0) ? 0 : (current_index + 1) % process_count;
    for (int k = 0; k < process_count; k++) {
        int i = (start + k) % process_count;
        if (process_table[i].state == PROCESS_READY) {
            pick_running(i);
            return;
        }
    }
}

static void schedule_hpf(void) {
    int best = -1;
    int best_prio = -2147483647;
    int best_pid = 2147483647;
    for (int i = 0; i < process_count; i++) {
        if (process_table[i].state != PROCESS_READY) {
            continue;
        }
        int p = process_table[i].priority;
        int pid = process_table[i].pid;
        if (p > best_prio || (p == best_prio && pid < best_pid)) {
            best_prio = p;
            best_pid = pid;
            best = i;
        }
    }
    if (best >= 0) {
        pick_running(best);
    }
}

void create_process(void (*func)(void)) {
    create_process_prio(func, 1);
}

void create_process_prio(void (*func)(void), int priority) {
    if (process_count >= MAX_PROCESSES || func == 0) {
        return;
    }

    process_table[process_count].pid = next_pid++;
    process_table[process_count].state = PROCESS_READY;
    process_table[process_count].priority = priority;
    process_table[process_count].entry_point = func;
    process_count++;
}

void schedule(void) {
    stat_schedule_calls++;
    if (process_count == 0) {
        return;
    }

    demote_running();

    switch (sched_policy) {
        case SCHED_RR:
            schedule_rr();
            break;
        case SCHED_FCFS:
            schedule_fcfs();
            break;
        case SCHED_HPF:
            schedule_hpf();
            break;
        default:
            schedule_rr();
            break;
    }
}

int get_process_count(void) {
    return process_count;
}

void process_set_policy(SchedPolicy p) {
    if (p <= SCHED_HPF) {
        sched_policy = p;
    }
}

SchedPolicy process_get_policy(void) {
    return sched_policy;
}

const char* process_policy_name(SchedPolicy p) {
    switch (p) {
        case SCHED_RR:
            return "RR";
        case SCHED_FCFS:
            return "FCFS";
        case SCHED_HPF:
            return "HPF";
        default:
            return "?";
    }
}

void process_reset_all(void) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        process_table[i].pid = 0;
        process_table[i].state = PROCESS_UNUSED;
        process_table[i].priority = 0;
        process_table[i].entry_point = 0;
    }
    process_count = 0;
    next_pid = 1;
    current_index = -1;
}

void sched_stats_reset(void) {
    stat_schedule_calls = 0;
    stat_context_switches = 0;
}

void sched_stats_print(void) {
    printf("\033[1;35mScheduler stats:\033[0m policy=\033[1;37m%s\033[0m\n",
        process_policy_name(sched_policy));
    printf("  schedule() calls:        \033[1;36m%u\033[0m\n", (unsigned int)stat_schedule_calls);
    printf("  context switches:       \033[1;36m%u\033[0m\n", (unsigned int)stat_context_switches);
}

unsigned long sched_get_invocations(void) {
    return stat_schedule_calls;
}

unsigned long sched_get_context_switches(void) {
    return stat_context_switches;
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
    printf("\033[1;36mPolicy: %s\033[0m  \033[1;36mPID  State     Prio  Entry\033[0m\n",
        process_policy_name(sched_policy));
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
    sched_stats_print();
    process_dump_all();
}
