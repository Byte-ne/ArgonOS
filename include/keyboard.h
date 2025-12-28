#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "types.h"

#define KEYBOARD_DATA_PORT    0x60
#define KEYBOARD_STATUS_PORT  0x64

#define KEY_ESCAPE    0x01
#define KEY_F1        0x3B
#define KEY_F2        0x3C
#define KEY_F3        0x3D
#define KEY_F4        0x3E
#define KEY_F5        0x3F
#define KEY_F6        0x40
#define KEY_F7        0x41
#define KEY_F8        0x42
#define KEY_F9        0x43
#define KEY_F10       0x44
#define KEY_F11       0x57
#define KEY_F12       0x58

#define KEY_ARROW_UP      0x48
#define KEY_ARROW_DOWN    0x50
#define KEY_ARROW_LEFT    0x4B
#define KEY_ARROW_RIGHT   0x4D

#define KEY_INSERT    0x52
#define KEY_DELETE    0x53
#define KEY_HOME      0x47
#define KEY_END       0x4F
#define KEY_PAGE_UP   0x49
#define KEY_PAGE_DOWN 0x51

#define KEY_LEFT_SHIFT   0x2A
#define KEY_RIGHT_SHIFT  0x36
#define KEY_CAPS_LOCK    0x3A
#define KEY_LEFT_CTRL    0x1D
#define KEY_LEFT_ALT     0x38

typedef struct {
    uint8_t shift_pressed;
    uint8_t ctrl_pressed;
    uint8_t alt_pressed;
    uint8_t caps_lock;
} keyboard_state_t;

void keyboard_init(void);
void keyboard_handler_c(void);
char keyboard_getchar(void);
int keyboard_has_key(void);
uint8_t keyboard_get_special_key(void);
int keyboard_has_special_key(void);
keyboard_state_t keyboard_get_state(void);

#endif