#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "types.h"
#include "task.h"

// Scheduler functions
void scheduler_init(void);
void scheduler_start(void);
void schedule(registers_t* regs);  // Called by timer interrupt

#endif