#ifndef PROCESS_H
#define PROCESS_H

#define MAX_PROCESSES 16

typedef enum {
    PROCESS_UNUSED = 0,
    PROCESS_READY = 1,
    PROCESS_RUNNING = 2,
    PROCESS_WAITING = 3
} ProcessState;

typedef struct {
    int pid;
    int state;
    int priority;
    void (*entry_point)(void);
} PCB;

void create_process(void (*func)(void));
void schedule(void);

#endif
