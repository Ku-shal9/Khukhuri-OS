#include "process.h"
#include "printf.h"

static PCB process_table[MAX_PROCESSES];
static int process_count = 0;
static int next_pid = 1;
static int current_index = -1;
static SchedPolicy sched_policy = SCHED_RR;

static unsigned long stat_schedule_calls = 0;
static unsigned long stat_context_switches = 0;
static unsigned long stat_completed_tasks = 0;

static int live_count(void) {
    int n = 0;
    for (int i = 0; i < process_count; i++) {
        if (process_table[i].state != PROCESS_UNUSED) {
            n++;
        }
    }
    return n;
}

static int live_thread_count(void) {
    int n = 0;
    for (int i = 0; i < process_count; i++) {
        if (process_table[i].state != PROCESS_UNUSED && process_table[i].is_thread) {
            n++;
        }
    }
    return n;
}

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

static void run_current_tick(void) {
    if (current_index < 0 || current_index >= process_count) {
        return;
    }
    PCB* p = &process_table[current_index];
    if (p->state != PROCESS_RUNNING) {
        return;
    }
    if (p->remaining_ticks > 0) {
        p->remaining_ticks--;
    }
    if (p->remaining_ticks <= 0) {
        p->state = PROCESS_UNUSED;
        stat_completed_tasks++;
        current_index = -1;
    }
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

static void schedule_srtf(void) {
    int best = -1;
    int best_remaining = 2147483647;
    int best_pid = 2147483647;
    for (int i = 0; i < process_count; i++) {
        if (process_table[i].state != PROCESS_READY) {
            continue;
        }
        int p = process_table[i].remaining_ticks;
        int pid = process_table[i].pid;
        if (p < best_remaining || (p == best_remaining && pid < best_pid)) {
            best_remaining = p;
            best_pid = pid;
            best = i;
        }
    }
    if (best >= 0) {
        pick_running(best);
    }
}

void create_process(void (*func)(void)) {
    create_process_burst(func, 5);
}

void create_process_prio(void (*func)(void), int priority) {
    create_process_burst(func, priority);
}

void create_process_burst(void (*func)(void), int burst_ticks) {
    if (process_count >= MAX_PROCESSES || func == 0) {
        return;
    }
    if (burst_ticks < 1) {
        burst_ticks = 1;
    }

    process_table[process_count].pid = next_pid++;
    process_table[process_count].state = PROCESS_READY;
    process_table[process_count].priority = burst_ticks;
    process_table[process_count].burst_ticks = burst_ticks;
    process_table[process_count].remaining_ticks = burst_ticks;
    process_table[process_count].is_thread = 0;
    process_table[process_count].parent_pid = 0;
    process_table[process_count].entry_point = func;
    process_count++;
}

void create_thread(void (*func)(void), int parent_pid, int burst_ticks) {
    if (process_count >= MAX_PROCESSES || func == 0) {
        return;
    }
    if (burst_ticks < 1) {
        burst_ticks = 1;
    }
    process_table[process_count].pid = next_pid++;
    process_table[process_count].state = PROCESS_READY;
    process_table[process_count].priority = burst_ticks;
    process_table[process_count].burst_ticks = burst_ticks;
    process_table[process_count].remaining_ticks = burst_ticks;
    process_table[process_count].is_thread = 1;
    process_table[process_count].parent_pid = parent_pid;
    process_table[process_count].entry_point = func;
    process_count++;
}

void schedule(void) {
    stat_schedule_calls++;
    if (process_count == 0) {
        return;
    }

    run_current_tick();
    demote_running();

    switch (sched_policy) {
        case SCHED_RR:
            schedule_rr();
            break;
        case SCHED_SRTF:
            schedule_srtf();
            break;
        default:
            schedule_rr();
            break;
    }
}

int get_process_count(void) {
    return live_count();
}

int get_thread_count(void) {
    return live_thread_count();
}

void process_set_policy(SchedPolicy p) {
    if (p <= SCHED_SRTF) {
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
        case SCHED_SRTF:
            return "SRTF";
        default:
            return "?";
    }
}

void process_reset_all(void) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        process_table[i].pid = 0;
        process_table[i].state = PROCESS_UNUSED;
        process_table[i].priority = 0;
        process_table[i].burst_ticks = 0;
        process_table[i].remaining_ticks = 0;
        process_table[i].is_thread = 0;
        process_table[i].parent_pid = 0;
        process_table[i].entry_point = 0;
    }
    process_count = 0;
    next_pid = 1;
    current_index = -1;
}

void sched_stats_reset(void) {
    stat_schedule_calls = 0;
    stat_context_switches = 0;
    stat_completed_tasks = 0;
}

void sched_stats_print(void) {
    printf("\033[1;35mScheduler stats:\033[0m policy=\033[1;37m%s\033[0m\n",
        process_policy_name(sched_policy));
    printf("  schedule() calls:        \033[1;36m%u\033[0m\n", (unsigned int)stat_schedule_calls);
    printf("  context switches:       \033[1;36m%u\033[0m\n", (unsigned int)stat_context_switches);
    printf("  completed tasks:        \033[1;36m%u\033[0m\n", (unsigned int)stat_completed_tasks);
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

static void print_int_raw(int v) {
    printf("%d", v);
}

static void print_padded_text(const char* s, int width) {
    int n = 0;
    while (s[n] != '\0') {
        putchar(s[n]);
        n++;
    }
    while (n < width) {
        putchar(' ');
        n++;
    }
}

static void print_padded_int(int v, int width) {
    char buf[16];
    int idx = 0;
    int x = v;
    if (x == 0) {
        buf[idx++] = '0';
    } else {
        if (x < 0) {
            buf[idx++] = '-';
            x = -x;
        }
        char tmp[16];
        int t = 0;
        while (x > 0 && t < 15) {
            tmp[t++] = (char)('0' + (x % 10));
            x /= 10;
        }
        while (t > 0) {
            buf[idx++] = tmp[--t];
        }
    }
    buf[idx] = '\0';
    print_padded_text(buf, width);
}

void process_dump_all(void) {
    printf("\033[1;36mPolicy: %s\033[0m\n", process_policy_name(sched_policy));
    printf("\033[1;36mPID  KIND    STATE     BURST  LEFT   PARENT  ENTRY\033[0m\n");
    printf("----------------------------------------------------------\n");
    if (live_count() == 0) {
        printf("\033[1;33m(no processes)\033[0m\n");
        return;
    }
    for (int i = 0; i < process_count; i++) {
        if (process_table[i].state == PROCESS_UNUSED) {
            continue;
        }
        putchar(' ');
        print_padded_int(process_table[i].pid, 4);
        print_padded_text(process_table[i].is_thread ? "thread" : "proc", 8);
        printf("\033[1;37m");
        print_padded_text(state_name(process_table[i].state), 10);
        printf("\033[0m");
        print_padded_int(process_table[i].burst_ticks, 7);
        print_padded_int(process_table[i].remaining_ticks, 7);
        print_padded_int(process_table[i].parent_pid, 8);
        printf("%p\n", (void*)process_table[i].entry_point);
    }
    if (current_index >= 0 && current_index < process_count) {
        printf("\033[1;32mCurrent slot index: ");
        print_int_raw(current_index);
        printf("\033[0m\n");
    }
    printf("\033[1;36mLive:\033[0m process=%d thread=%d total=%d\n",
        live_count() - live_thread_count(), live_thread_count(), live_count());
}

void process_schedule_times(int times) {
    for (int i = 0; i < times; i++) {
        schedule();
    }
    printf("\033[1;35mAfter %d schedule() step(s):\033[0m\n", times);
    sched_stats_print();
    process_dump_all();
}
