#include "scheduler.h"
#include "task.h"
#include "timer.h"
#include "string.h"
#include "paging.h"

static uint8_t scheduler_enabled = 0;

// External functions from task.c
extern task_t* task_get_by_index(uint32_t index);
extern uint32_t task_get_current_index(void);
extern void task_set_current_index(uint32_t index);

void scheduler_init(void) {
    task_init();
    scheduler_enabled = 0;  // Don't start scheduling yet
}

void scheduler_start(void) {
    scheduler_enabled = 1;
}

// Round-robin scheduler (called by timer interrupt)
void schedule(registers_t* regs) {
    if (!scheduler_enabled) {
        return;  // Scheduling not started yet
    }
    
    // Save current task's registers
    uint32_t current_idx = task_get_current_index();
    task_t* current = task_get_by_index(current_idx);
    
    if (current && current->state == TASK_RUNNING) {
        // Save CPU state
        current->regs.eax = regs->eax;
        current->regs.ebx = regs->ebx;
        current->regs.ecx = regs->ecx;
        current->regs.edx = regs->edx;
        current->regs.esi = regs->esi;
        current->regs.edi = regs->edi;
        current->regs.ebp = regs->ebp;
        current->regs.esp = regs->esp;
        current->regs.eip = regs->eip;
        current->regs.eflags = regs->eflags;
    }
    
    // Wake up sleeping tasks
    uint32_t current_ticks = timer_get_ticks();
    for (uint32_t i = 0; i < MAX_TASKS; i++) {
        task_t* t = task_get_by_index(i);
        if (t && t->state == TASK_SLEEPING) {
            if (current_ticks >= t->sleep_until) {
                t->state = TASK_RUNNING;
            }
        }
    }
    
    // Find next runnable task (round-robin)
    uint32_t next_idx = current_idx;
    uint32_t checked = 0;
    
    do {
        next_idx = (next_idx + 1) % MAX_TASKS;
        checked++;
        
        task_t* next = task_get_by_index(next_idx);
        if (next && next->state == TASK_RUNNING && next->id != 0) {
            // Found a runnable task
            task_set_current_index(next_idx);
            
            // Switch to task's page directory (virtual memory space)
            if (next->page_directory) {
                paging_switch_directory(next->page_directory);
            }
            
            // Restore its registers
            regs->eax = next->regs.eax;
            regs->ebx = next->regs.ebx;
            regs->ecx = next->regs.ecx;
            regs->edx = next->regs.edx;
            regs->esi = next->regs.esi;
            regs->edi = next->regs.edi;
            regs->ebp = next->regs.ebp;
            regs->esp = next->regs.esp;
            regs->eip = next->regs.eip;
            regs->eflags = next->regs.eflags;
            
            return;
        }
        
    } while (checked < MAX_TASKS);
    
    // No runnable tasks, stay in kernel
    task_set_current_index(0);
    
    // Switch back to kernel page directory
    task_t* kernel = task_get_by_index(0);
    if (kernel && kernel->page_directory) {
        paging_switch_directory(kernel->page_directory);
    }
}