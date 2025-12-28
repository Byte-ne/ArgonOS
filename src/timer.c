#include "timer.h"
#include "io.h"

static volatile uint32_t ticks = 0;

void timer_handler_c(void) {
    ticks++;
}

void timer_init(uint32_t frequency) {
    ticks = 0;
    
    uint32_t divisor = PIT_FREQUENCY / frequency;
    
    outb(0x43, 0x36);
    
    uint8_t low = (uint8_t)(divisor & 0xFF);
    uint8_t high = (uint8_t)((divisor >> 8) & 0xFF);
    
    outb(0x40, low);
    outb(0x40, high);
}

uint32_t timer_get_ticks(void) {
    return ticks;
}

uint32_t timer_get_seconds(void) {
    return ticks / TIMER_FREQUENCY;
}

void timer_wait(uint32_t wait_ticks) {
    uint32_t target = ticks + wait_ticks;
    while (ticks < target) {
        __asm__ volatile ("hlt");
    }
}