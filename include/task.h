#ifndef TASK_H
#define TASK_H

#include "types.h"

#define MAX_TASKS 16
#define TASK_STACK_SIZE 4096

typedef enum {
    TASK_READY = 0,
    TASK_RUNNING,
    TASK_BLOCKED,
    TASK_DEAD
} task_state_t;

typedef struct {
    uint32_t eax, ebx, ecx, edx;
    uint32_t esi, edi;
    uint32_t esp, ebp;
    uint32_t eip;
    uint32_t eflags;
} task_registers_t;

typedef struct {
    uint32_t id;
    char name[32];
    task_state_t state;
    task_registers_t regs;
    uint8_t* stack;
    uint32_t cpu_time;
} task_t;

void task_init(void);
uint32_t task_create(const char* name, void (*entry_point)(void));
void task_exit(void);
void task_kill(uint32_t task_id);
task_t* task_get_current(void);
task_t* task_get_by_id(uint32_t id);
uint32_t task_get_count(void);
void task_list_all(void);

#endif