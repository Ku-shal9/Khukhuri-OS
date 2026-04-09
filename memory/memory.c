#include "memory.h"

void memory_init(void) {
    /* Actual implementation will be added in a later milestone. */
}

void* os_malloc(int size) {
    (void)size;
    return 0;
}

void os_free(void* ptr) {
    (void)ptr;
}

/* Compatibility wrappers for the fixed interface contract in setup docs. */
void* malloc(int size) {
    return os_malloc(size);
}

void free(void* ptr) {
    os_free(ptr);
}