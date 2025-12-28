#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "types.h"
#include "task.h"

void scheduler_init(void);
void scheduler_switch_task(task_registers_t* current_regs);
void scheduler_start(void);

#endif