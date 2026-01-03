#include "task.h"
#include "memory.h"
#include "string.h"
#include "timer.h"
#include "kernel.h"
#include "paging.h"

static task_t tasks[MAX_TASKS];
static uint32_t next_task_id = 1;  // 0 reserved for kernel
static uint32_t current_task_id = 0;

void task_init(void) {
    memset(tasks, 0, sizeof(tasks));
    
    // Initialize kernel task (task 0)
    tasks[0].id = 0;
    strcpy(tasks[0].name, "kernel");
    tasks[0].state = TASK_RUNNING;
    tasks[0].page_directory = paging_get_current_directory();  // Use kernel's page directory
    current_task_id = 0;
}

uint32_t task_create(const char* name, void (*entry_point)(void)) {
    // Find free task slot
    uint32_t task_id = 0;
    for (uint32_t i = 1; i < MAX_TASKS; i++) {
        if (tasks[i].state == TASK_DEAD || tasks[i].id == 0) {
            task_id = i;
            break;
        }
    }
    
    if (task_id == 0) {
        return 0;  // No free slots
    }
    
    // Initialize task
    task_t* task = &tasks[task_id];
    task->id = next_task_id++;
    strcpy(task->name, name);
    task->state = TASK_RUNNING;
    task->sleep_until = 0;
    
    // Create own virtual address space
    task->page_directory = paging_create_address_space();
    
    // Setup stack (grows downward)
    uint32_t* stack_top = &task->stack[TASK_STACK_SIZE / 4 - 1];
    
    // Push entry point address on stack
    stack_top--;
    *stack_top = (uint32_t)task_exit;  // Return address (when task returns)
    
    // Initialize registers
    memset(&task->regs, 0, sizeof(registers_t));
    task->regs.eip = (uint32_t)entry_point;  // Start here
    task->regs.esp = (uint32_t)stack_top;    // Stack pointer
    task->regs.eflags = 0x202;               // Interrupts enabled
    
    return task->id;
}

void task_exit(void) {
    tasks[current_task_id].state = TASK_DEAD;
    while(1) {
        asm volatile("hlt");  // Wait for scheduler to remove us
    }
}

void task_kill(uint32_t task_id) {
    for (uint32_t i = 0; i < MAX_TASKS; i++) {
        if (tasks[i].id == task_id) {
            tasks[i].state = TASK_DEAD;
            return;
        }
    }
}

void task_sleep(uint32_t ticks) {
    tasks[current_task_id].state = TASK_SLEEPING;
    tasks[current_task_id].sleep_until = timer_get_ticks() + ticks;
}

task_t* task_get_current(void) {
    return &tasks[current_task_id];
}

task_t* task_get_by_id(uint32_t id) {
    for (uint32_t i = 0; i < MAX_TASKS; i++) {
        if (tasks[i].id == id && tasks[i].state != TASK_DEAD) {
            return &tasks[i];
        }
    }
    return 0;
}

uint32_t task_count_active(void) {
    uint32_t count = 0;
    for (uint32_t i = 0; i < MAX_TASKS; i++) {
        if (tasks[i].state != TASK_DEAD && tasks[i].id != 0) {
            count++;
        }
    }
    return count;
}

// Get task by index (for scheduler)
task_t* task_get_by_index(uint32_t index) {
    if (index < MAX_TASKS) {
        return &tasks[index];
    }
    return 0;
}

// Get current task index
uint32_t task_get_current_index(void) {
    return current_task_id;
}

// Set current task index
void task_set_current_index(uint32_t index) {
    current_task_id = index;
}

// Demo task: Counter
void task_counter(void) {
    uint32_t count = 0;
    while(1) {
        count++;
        task_sleep(100);  // Sleep 1 second
    }
}

// Demo task: Clock (shows current uptime)
void task_clock(void) {
    while(1) {
        task_sleep(100);  // Update every second
    }
}

// Demo task: Spinner
void task_spinner(void) {
    const char spinner[] = {'|', '/', '-', '\\'};
    uint32_t i = 0;
    while(1) {
        (void)spinner;  // Suppress unused warning (not displayed yet)
        i = (i + 1) % 4;
        task_sleep(25);  // Spin 4 times per second
    }
}