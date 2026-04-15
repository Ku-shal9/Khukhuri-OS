#ifndef MEMORY_H
#define MEMORY_H

typedef struct MemoryBlock {
    int size;
    int free;
    struct MemoryBlock* next;
} MemoryBlock;

void memory_init(void);
void* os_malloc(int size);
void os_free(void* ptr);
void* malloc(int size);
void free(void* ptr);

#endif
