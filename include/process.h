#ifndef PROCESS_H
#define PROCESS_H

#define MAX_PROCESSES 16

typedef enum {
    PROCESS_UNUSED = 0,
    PROCESS_READY = 1,
    PROCESS_RUNNING = 2,
    PROCESS_WAITING = 3
} ProcessState;

typedef enum {
    SCHED_RR = 0,
    SCHED_FCFS = 1,
    SCHED_HPF = 2
} SchedPolicy;

typedef struct {
    int pid;
    int state;
    int priority;
    void (*entry_point)(void);
} PCB;

void create_process(void (*func)(void));
void create_process_prio(void (*func)(void), int priority);
void schedule(void);
int get_process_count(void);
void process_dump_all(void);
void process_schedule_times(int times);

void process_set_policy(SchedPolicy p);
SchedPolicy process_get_policy(void);
const char* process_policy_name(SchedPolicy p);

void process_reset_all(void);
void sched_stats_reset(void);
void sched_stats_print(void);
unsigned long sched_get_invocations(void);
unsigned long sched_get_context_switches(void);

#endif
