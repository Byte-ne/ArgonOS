#ifndef TIMER_H
#define TIMER_H

#include "types.h"

#define PIT_FREQUENCY 1193182
#define TIMER_FREQUENCY 100

void timer_init(uint32_t frequency);
uint32_t timer_get_ticks(void);
uint32_t timer_get_seconds(void);
void timer_wait(uint32_t ticks);
void timer_handler_c(void);

#endif