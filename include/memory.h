#ifndef MEMORY_H
#define MEMORY_H

typedef struct {
    int size;
    int free;
} MemoryBlock;

void memory_init(void);

/* Milestone 1 stubs required by your module split. */
void* os_malloc(int size);
void os_free(void* ptr);

/* Compatibility API from the project contract. */
void* malloc(int size);
void free(void* ptr);

#endif
