#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

#define PAGE_SIZE 4096
#define HEAP_START 0x00200000
#define HEAP_SIZE (1024 * 1024)

typedef struct mem_block {
    uint32_t size;
    uint8_t is_free;
    struct mem_block* next;
} mem_block_t;

void memory_init(void);
void* kmalloc(uint32_t size);
void kfree(void* ptr);
uint32_t memory_get_total(void);
uint32_t memory_get_used(void);
uint32_t memory_get_free(void);
void memory_print_blocks(void);

#endif