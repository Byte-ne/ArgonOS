#include "task.h"
#include "memory.h"
#include "string.h"

static task_t tasks[MAX_TASKS];
static uint32_t current_task_id = 0;
static uint32_t next_task_id = 1;
static uint32_t task_count = 0;

void task_init(void) {
    memset(tasks, 0, sizeof(tasks));
    
    tasks[0].id = 0;
    strcpy(tasks[0].name, "kernel");
    tasks[0].state = TASK_RUNNING;
    tasks[0].stack = NULL;
    tasks[0].cpu_time = 0;
    
    current_task_id = 0;
    next_task_id = 1;
    task_count = 1;
}

uint32_t task_create(const char* name, void (*entry_point)(void)) {
    if (task_count >= MAX_TASKS) {
        return 0;
    }
    
    uint32_t id = next_task_id++;
    int slot = -1;
    
    for (int i = 0; i < MAX_TASKS; i++) {
        if (tasks[i].state == TASK_DEAD || tasks[i].id == 0) {
            slot = i;
            break;
        }
    }
    
    if (slot == -1) {
        for (int i = 1; i < MAX_TASKS; i++) {
            if (tasks[i].id == 0) {
                slot = i;
                break;
            }
        }
    }
    
    if (slot == -1) return 0;
    
    tasks[slot].id = id;
    strncpy(tasks[slot].name, name, 31);
    tasks[slot].name[31] = '\0';
    tasks[slot].state = TASK_READY;
    
    tasks[slot].stack = (uint8_t*)kmalloc(TASK_STACK_SIZE);
    if (!tasks[slot].stack) {
        return 0;
    }
    
    memset(&tasks[slot].regs, 0, sizeof(task_registers_t));
    
    tasks[slot].regs.esp = (uint32_t)(tasks[slot].stack + TASK_STACK_SIZE - 4);
    tasks[slot].regs.ebp = tasks[slot].regs.esp;
    tasks[slot].regs.eip = (uint32_t)entry_point;
    tasks[slot].regs.eflags = 0x202;
    
    tasks[slot].cpu_time = 0;
    
    task_count++;
    
    return id;
}

void task_exit(void) {
    tasks[current_task_id].state = TASK_DEAD;
    task_count--;
    
    __asm__ volatile("int $0x20");
}

void task_kill(uint32_t task_id) {
    for (int i = 0; i < MAX_TASKS; i++) {
        if (tasks[i].id == task_id && tasks[i].state != TASK_DEAD) {
            if (tasks[i].stack) {
                kfree(tasks[i].stack);
                tasks[i].stack = NULL;
            }
            tasks[i].state = TASK_DEAD;
            task_count--;
            return;
        }
    }
}

task_t* task_get_current(void) {
    for (int i = 0; i < MAX_TASKS; i++) {
        if (tasks[i].id == current_task_id) {
            return &tasks[i];
        }
    }
    return &tasks[0];
}

task_t* task_get_by_id(uint32_t id) {
    for (int i = 0; i < MAX_TASKS; i++) {
        if (tasks[i].id == id) {
            return &tasks[i];
        }
    }
    return NULL;
}

uint32_t task_get_count(void) {
    return task_count;
}

void task_list_all(void) {
    for (int i = 0; i < MAX_TASKS; i++) {
        if (tasks[i].state != TASK_DEAD && tasks[i].id != 0) {
            // Callback for display
        }
    }
}