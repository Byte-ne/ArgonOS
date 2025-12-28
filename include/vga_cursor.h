#ifndef VGA_CURSOR_H
#define VGA_CURSOR_H

#include "types.h"

void vga_cursor_enable(uint8_t cursor_start, uint8_t cursor_end);
void vga_cursor_disable(void);
void vga_cursor_set_position(uint16_t x, uint16_t y);
void vga_cursor_update_position(void);

#endif