#include "memory.h"
#include "string.h"

static uint8_t heap[HEAP_SIZE];
static mem_block_t* heap_start = NULL;
static uint32_t total_allocated = 0;

void memory_init(void) {
    heap_start = (mem_block_t*)heap;
    heap_start->size = HEAP_SIZE - sizeof(mem_block_t);
    heap_start->is_free = 1;
    heap_start->next = NULL;
    total_allocated = 0;
}

void* kmalloc(uint32_t size) {
    if (size == 0) return NULL;
    
    size = (size + 3) & ~3;
    
    mem_block_t* current = heap_start;
    
    while (current) {
        if (current->is_free && current->size >= size) {
            if (current->size >= size + sizeof(mem_block_t) + 16) {
                mem_block_t* new_block = (mem_block_t*)((uint8_t*)current + sizeof(mem_block_t) + size);
                new_block->size = current->size - size - sizeof(mem_block_t);
                new_block->is_free = 1;
                new_block->next = current->next;
                
                current->size = size;
                current->next = new_block;
            }
            
            current->is_free = 0;
            total_allocated += current->size;
            
            return (void*)((uint8_t*)current + sizeof(mem_block_t));
        }
        
        current = current->next;
    }
    
    return NULL;
}

void kfree(void* ptr) {
    if (!ptr) return;
    
    mem_block_t* block = (mem_block_t*)((uint8_t*)ptr - sizeof(mem_block_t));
    block->is_free = 1;
    total_allocated -= block->size;
    
    mem_block_t* current = heap_start;
    while (current && current->next) {
        if (current->is_free && current->next->is_free) {
            current->size += sizeof(mem_block_t) + current->next->size;
            current->next = current->next->next;
        } else {
            current = current->next;
        }
    }
}

uint32_t memory_get_total(void) {
    return HEAP_SIZE;
}

uint32_t memory_get_used(void) {
    return total_allocated;
}

uint32_t memory_get_free(void) {
    return HEAP_SIZE - total_allocated - sizeof(mem_block_t);
}

void memory_print_blocks(void) {
    mem_block_t* current = heap_start;
    int block_num = 0;
    
    while (current) {
        block_num++;
        current = current->next;
    }
}