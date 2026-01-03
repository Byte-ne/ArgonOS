#include "idt.h"
#include "pic.h"
#include "types.h"

// External ISR/IRQ handlers from idt_asm.asm
extern void isr0(void);
extern void isr1(void);
extern void isr14(void);  // Page fault

extern void irq0(void);
extern void irq1(void);
extern void irq2(void);
extern void irq3(void);
extern void irq4(void);
extern void irq5(void);
extern void irq6(void);
extern void irq7(void);
extern void irq8(void);
extern void irq9(void);
extern void irq10(void);
extern void irq11(void);
extern void irq12(void);
extern void irq13(void);
extern void irq14(void);
extern void irq15(void);

struct idt_entry idt_entries[IDT_ENTRIES];
struct idt_ptr idtp;

extern void idt_load(uint32_t);

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt_entries[num].base_lo = base & 0xFFFF;
    idt_entries[num].base_hi = (base >> 16) & 0xFFFF;
    idt_entries[num].sel = sel;
    idt_entries[num].always0 = 0;
    idt_entries[num].flags = flags;
}

void idt_init(void) {
    idtp.limit = sizeof(struct idt_entry) * IDT_ENTRIES - 1;
    idtp.base = (uint32_t)&idt_entries;
    
    // Clear all entries
    for (int i = 0; i < IDT_ENTRIES; i++) {
        idt_set_gate(i, 0, 0, 0);
    }
    
    // Remap PIC
    pic_remap(0x20, 0x28);
    
    // Set up exception handlers
    idt_set_gate(0, (uint32_t)isr0, 0x08, 0x8E);   // Divide by zero
    idt_set_gate(1, (uint32_t)isr1, 0x08, 0x8E);   // Debug
    idt_set_gate(14, (uint32_t)isr14, 0x08, 0x8E); // Page fault
    
    // Set up IRQ handlers (hardware interrupts)
    idt_set_gate(32, (uint32_t)irq0, 0x08, 0x8E);  // Timer
    idt_set_gate(33, (uint32_t)irq1, 0x08, 0x8E);  // Keyboard
    
    // Load IDT
    idt_load((uint32_t)&idtp);
    
    // Enable interrupts for timer and keyboard
    pic_clear_mask(0);  // Timer
    pic_clear_mask(1);  // Keyboard
    
    // Enable interrupts
    __asm__ volatile ("sti");
}