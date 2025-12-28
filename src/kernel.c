#include "kernel.h"
#include "idt.h"
#include "pic.h"
#include "keyboard.h"
#include "timer.h"
#include "memory.h"
#include "command.h"
#include "string.h"
#include "lineedit.h"

static uint16_t* terminal_buffer;
static uint32_t terminal_row;
static uint32_t terminal_column;
static uint8_t terminal_color;

extern void commands_register_all(void);

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
    return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t) uc | (uint16_t) color << 8;
}

void terminal_initialize(void) {
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK);
    terminal_buffer = (uint16_t*) VGA_MEMORY;
    
    for (uint32_t y = 0; y < VGA_HEIGHT; y++) {
        for (uint32_t x = 0; x < VGA_WIDTH; x++) {
            const uint32_t index = y * VGA_WIDTH + x;
            terminal_buffer[index] = vga_entry(' ', terminal_color);
        }
    }
}

void terminal_setcolor(uint8_t color) {
    terminal_color = color;
}

void terminal_putentryat(char c, uint8_t color, uint32_t x, uint32_t y) {
    const uint32_t index = y * VGA_WIDTH + x;
    terminal_buffer[index] = vga_entry(c, color);
}

void terminal_scroll(void) {
    for (uint32_t y = 0; y < VGA_HEIGHT - 1; y++) {
        for (uint32_t x = 0; x < VGA_WIDTH; x++) {
            terminal_buffer[y * VGA_WIDTH + x] = terminal_buffer[(y + 1) * VGA_WIDTH + x];
        }
    }
    
    for (uint32_t x = 0; x < VGA_WIDTH; x++) {
        terminal_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', terminal_color);
    }
    
    terminal_row = VGA_HEIGHT - 1;
}

void terminal_putchar(char c) {
    if (c == '\r') {
        terminal_column = 0;
        return;
    }
    
    if (c == '\n') {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT) {
            terminal_scroll();
        }
        return;
    }
    
    if (c == '\b') {
        if (terminal_column > 0) {
            terminal_column--;
            terminal_putentryat(' ', terminal_color, terminal_column, terminal_row);
        }
        return;
    }
    
    terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
    
    if (++terminal_column == VGA_WIDTH) {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT) {
            terminal_scroll();
        }
    }
}

void terminal_write(const char* data, uint32_t size) {
    for (uint32_t i = 0; i < size; i++)
        terminal_putchar(data[i]);
}

void terminal_writestring(const char* data) {
    terminal_write(data, strlen(data));
}

void terminal_clear(void) {
    terminal_initialize();
}

void print_prompt(void) {
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    terminal_writestring("C:\\Argon> ");
    terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
}

void kernel_main(void) {
    terminal_initialize();
    
    terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
    terminal_writestring("Argon OS [Version 0.2.3]\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    terminal_writestring("(c) Argon Corporation. All rights reserved.\n\n");
    
    idt_init();
    keyboard_init();
    timer_init(TIMER_FREQUENCY);
    memory_init();
    command_init();
    lineedit_init();
    commands_register_all();
    
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN, VGA_BLACK));
    terminal_writestring("Line editor enabled. Use arrow keys!\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    terminal_writestring("Type 'help.commands' for available commands.\n\n");
    
    print_prompt();
    
    while (1) {
        if (keyboard_has_special_key()) {
            uint8_t special = keyboard_get_special_key();
            lineedit_handle_special_key(special);
        }
        
        if (keyboard_has_key()) {
            char c = keyboard_getchar();
            lineedit_handle_key(c);
            
            if (lineedit_is_complete()) {
                char* line = lineedit_get_line();
                
                if (strlen(line) > 0) {
                    command_execute(line);
                }
                
                lineedit_reset();
                print_prompt();
            }
        }
        
        __asm__ volatile ("hlt");
    }
}