#include "isr.h"
#include "pic.h"
#include "kernel.h"

extern void keyboard_handler_c(void);
extern void timer_handler_c(void);

void isr_handler(registers_t* regs) {
    if (regs->int_no == 32) {
        timer_handler_c();
        pic_send_eoi(0);
    }
    else if (regs->int_no == 33) {
        keyboard_handler_c();
        pic_send_eoi(1);
    }
}