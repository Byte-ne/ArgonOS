#include "scheduler.h"
#include "task.h"

extern task_t tasks[];
extern uint32_t current_task_id;

static uint8_t scheduler_enabled = 0;

void scheduler_init(void) {
    scheduler_enabled = 0;
}

void scheduler_start(void) {
    scheduler_enabled = 1;
}

void scheduler_switch_task(task_registers_t* current_regs) {
    if (!scheduler_enabled) {
        return;
    }
    
    task_t* current = task_get_current();
    if (!current) return;
    
    if (current->state == TASK_RUNNING) {
        current->state = TASK_READY;
    }
    
    if (current_regs && current->state != TASK_DEAD) {
        current->regs = *current_regs;
    }
    
    int next_slot = -1;
    int current_slot = -1;
    
    for (int i = 0; i < MAX_TASKS; i++) {
        if (tasks[i].id == current_task_id) {
            current_slot = i;
            break;
        }
    }
    
    if (current_slot == -1) current_slot = 0;
    
    for (int i = 1; i <= MAX_TASKS; i++) {
        int idx = (current_slot + i) % MAX_TASKS;
        if (tasks[idx].state == TASK_READY || 
            (tasks[idx].state == TASK_RUNNING && tasks[idx].id != current_task_id)) {
            next_slot = idx;
            break;
        }
    }
    
    if (next_slot == -1) {
        if (current->state != TASK_DEAD) {
            current->state = TASK_RUNNING;
            return;
        }
        next_slot = 0;
    }
    
    current_task_id = tasks[next_slot].id;
    tasks[next_slot].state = TASK_RUNNING;
    tasks[next_slot].cpu_time++;
    
    if (current_regs) {
        *current_regs = tasks[next_slot].regs;
    }
}