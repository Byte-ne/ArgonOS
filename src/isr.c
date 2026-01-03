#include "isr.h"
#include "pic.h"
#include "scheduler.h"
#include "kernel.h"
#include "task.h"
#include "string.h"

// External handlers from your existing code
extern void keyboard_handler_c(void);
extern void timer_handler_c(void);

// Interrupt handler dispatcher (for exceptions)
void isr_handler(uint32_t int_no) {
    if (int_no == 14) {
        // Page fault!
        uint32_t faulting_address;
        __asm__ volatile("mov %%cr2, %0" : "=r"(faulting_address));
        
        task_t* current = task_get_current();
        
        terminal_setcolor(vga_entry_color(VGA_RED, VGA_BLACK));
        terminal_writestring("\n[PAGE FAULT] ");
        
        if (current && current->id != 0) {
            terminal_writestring("Task '");
            terminal_writestring(current->name);
            terminal_writestring("' (ID ");
            char id_str[12];
            itoa(current->id, id_str);
            terminal_writestring(id_str);
            terminal_writestring(") ");
        }
        
        terminal_writestring("accessed invalid address 0x");
        char addr_str[12];
        itoa(faulting_address, addr_str);
        terminal_writestring(addr_str);
        terminal_writestring("\n");
        
        terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
        
        // Kill the offending task (if not kernel)
        if (current && current->id != 0) {
            terminal_writestring("[KERNEL] Killing task\n\n");
            task_kill(current->id);
        } else {
            terminal_writestring("[KERNEL PANIC] Page fault in kernel!\n");
            while(1) { __asm__ volatile("hlt"); }
        }
    }
}

// IRQ handler dispatcher (for hardware interrupts)
void irq_handler(uint32_t int_no, registers_t* regs) {
    // Handle specific IRQs
    if (int_no == 32) {
        // Timer interrupt (IRQ0)
        timer_handler_c();
        
        // Call scheduler for task switching
        schedule(regs);
    }
    else if (int_no == 33) {
        // Keyboard interrupt (IRQ1)
        keyboard_handler_c();
    }
    
    // Send EOI to PIC
    pic_send_eoi(int_no - 32);
}