#include "kernel.h"
#include "idt.h"
#include "pic.h"
#include "keyboard.h"
#include "timer.h"
#include "memory.h"
#include "command.h"
#include "string.h"
#include "lineedit.h"
#include "vga_cursor.h"
#include "task.h"
#include "scheduler.h"

static uint16_t* terminal_buffer;
static uint32_t terminal_row;
static uint32_t terminal_column;
static uint8_t terminal_color;

extern void commands_register_all(void);

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
    
    vga_cursor_enable(14, 15);
}

void terminal_setcolor(uint8_t color) {
    terminal_color = color;
}

uint8_t terminal_get_color(void) {
    return terminal_color;
}

uint32_t terminal_get_row(void) {
    return terminal_row;
}

void terminal_putentryat(char c, uint8_t color, uint32_t x, uint32_t y) {
    const uint32_t index = y * VGA_WIDTH + x;
    terminal_buffer[index] = vga_entry(c, color);
}

void terminal_putchar_at(char c, uint8_t color, uint32_t x, uint32_t y) {
    terminal_putentryat(c, color, x, y);
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
        vga_cursor_set_position(terminal_column, terminal_row);
        return;
    }
    
    if (c == '\n') {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT) {
            terminal_scroll();
        }
        vga_cursor_set_position(terminal_column, terminal_row);
        return;
    }
    
    if (c == '\b') {
        if (terminal_column > 0) {
            terminal_column--;
            terminal_putentryat(' ', terminal_color, terminal_column, terminal_row);
            vga_cursor_set_position(terminal_column, terminal_row);
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
    
    vga_cursor_set_position(terminal_column, terminal_row);
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
    terminal_writestring("Argon OS [Version 0.2.3 Final]\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    terminal_writestring("(c) Argon Corporation. All rights reserved.\n\n");
    
    idt_init();
    keyboard_init();
    timer_init(TIMER_FREQUENCY);
    memory_init();
    command_init();
    lineedit_init();
    task_init();
    scheduler_init();
    commands_register_all();
    
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN, VGA_BLACK));
    terminal_writestring("Multitasking enabled! Try 'process.create counter'\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    terminal_writestring("Type 'help.commands' for available commands.\n\n");
    
    scheduler_start();
    
    print_prompt();
    
    while (1) {
        if (keyboard_has_special_key()) {
            uint8_t special = keyboard_get_special_key();
            lineedit_handle_special_key(special);
        }
        
        if (keyboard_has_key()) {
            keyboard_state_t state = keyboard_get_state();
            char c = keyboard_getchar();
            
            if (state.ctrl_pressed) {
                lineedit_handle_ctrl_key(c);
            } else {
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
        }
        
        __asm__ volatile ("hlt");
    }
}