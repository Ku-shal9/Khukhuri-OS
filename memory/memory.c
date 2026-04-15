#include "memory.h"

static unsigned char heap_area[64 * 1024];
static MemoryBlock* head = 0;

static int align8(int size) {
    return (size + 7) & ~7;
}

void memory_init(void) {
    head = (MemoryBlock*)heap_area;
    head->size = (int)(sizeof(heap_area) - sizeof(MemoryBlock));
    head->free = 1;
    head->next = 0;
}

void* os_malloc(int size) {
    if (size <= 0) {
        return 0;
    }
    if (head == 0) {
        memory_init();
    }

    int request = align8(size);
    MemoryBlock* current = head;
    while (current != 0) {
        if (current->free && current->size >= request) {
            if (current->size >= request + (int)sizeof(MemoryBlock) + 8) {
                MemoryBlock* split = (MemoryBlock*)((unsigned char*)current + sizeof(MemoryBlock) + request);
                split->size = current->size - request - (int)sizeof(MemoryBlock);
                split->free = 1;
                split->next = current->next;
                current->next = split;
                current->size = request;
            }
            current->free = 0;
            return (unsigned char*)current + sizeof(MemoryBlock);
        }
        current = current->next;
    }
    return 0;
}

void os_free(void* ptr) {
    if (ptr == 0) {
        return;
    }

    MemoryBlock* block = (MemoryBlock*)((unsigned char*)ptr - sizeof(MemoryBlock));
    block->free = 1;

    MemoryBlock* current = head;
    while (current != 0 && current->next != 0) {
        if (current->free && current->next->free) {
            current->size += (int)sizeof(MemoryBlock) + current->next->size;
            current->next = current->next->next;
            continue;
        }
        current = current->next;
    }
}

void* malloc(int size) {
    return os_malloc(size);
}

void free(void* ptr) {
    os_free(ptr);
}