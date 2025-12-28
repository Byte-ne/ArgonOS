#include "isr.h"
#include "pic.h"
#include "kernel.h"
#include "scheduler.h"

extern void keyboard_handler_c(void);
extern void timer_handler_c(void);
extern void task_switch_context(void);

static uint32_t timer_tick_count = 0;

void isr_handler(registers_t* regs) {
    if (regs->int_no == 32) {
        timer_handler_c();
        
        timer_tick_count++;
        if (timer_tick_count % 10 == 0) {
            scheduler_switch_task((task_registers_t*)regs);
        }
        
        pic_send_eoi(0);
    }
    else if (regs->int_no == 33) {
        keyboard_handler_c();
        pic_send_eoi(1);
    }
}