#include "task.h"
#include "kernel.h"
#include "timer.h"
#include "string.h"

static volatile uint32_t counter_value = 0;

void task_counter(void) {
    while (1) {
        counter_value++;
        
        for (volatile int i = 0; i < 100000; i++);
    }
}

void task_spinner(void) {
    const char spinner[] = "|/-\\";
    int idx = 0;
    
    while (1) {
        idx = (idx + 1) % 4;
        
        for (volatile int i = 0; i < 200000; i++);
    }
}

void task_clock(void) {
    uint32_t last_second = 0;
    
    while (1) {
        uint32_t current_second = timer_get_seconds();
        if (current_second != last_second) {
            last_second = current_second;
        }
        
        for (volatile int i = 0; i < 100000; i++);
    }
}

uint32_t get_counter_value(void) {
    return counter_value;
}