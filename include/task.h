#ifndef TASK_H
#define TASK_H

#include "types.h"
#include "isr.h"  // For registers_t

// Forward declaration
typedef struct page_directory page_directory_t;

#define MAX_TASKS 32
#define TASK_STACK_SIZE 4096

// Task states
typedef enum {
    TASK_RUNNING,   // Currently executing or ready to run
    TASK_SLEEPING,  // Waiting for timer
    TASK_DEAD       // Finished, can be cleaned up
} task_state_t;

// Task control block
typedef struct {
    uint32_t id;                    // Task ID (0 = kernel/idle)
    char name[32];                  // Task name
    task_state_t state;             // Current state
    registers_t regs;               // Saved registers
    uint32_t stack[TASK_STACK_SIZE / 4];  // Task stack (4KB)
    uint32_t sleep_until;           // Wake up time (ticks)
    page_directory_t* page_directory;  // Virtual memory space (Phase 2.5)
} task_t;

// Task management functions
void task_init(void);
uint32_t task_create(const char* name, void (*entry_point)(void));
void task_exit(void);
void task_kill(uint32_t task_id);
void task_sleep(uint32_t ticks);
task_t* task_get_current(void);
task_t* task_get_by_id(uint32_t id);
task_t* task_get_by_index(uint32_t index);  // ADD THIS
uint32_t task_count_active(void);

// Demo tasks (for testing)
void task_counter(void);
void task_clock(void);
void task_spinner(void);

#endif