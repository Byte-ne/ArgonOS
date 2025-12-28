#include "keyboard.h"
#include "io.h"

static const char scancode_to_ascii_lower[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static const char scancode_to_ascii_upper[128] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '', '?', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

#define BUFFER_SIZE 256
static char key_buffer[BUFFER_SIZE];
static volatile int buffer_start = 0;
static volatile int buffer_end = 0;

static uint8_t special_key_buffer[BUFFER_SIZE];
static volatile int special_start = 0;
static volatile int special_end = 0;

static keyboard_state_t kbd_state = {0, 0, 0, 0};

void keyboard_init(void) {
    buffer_start = 0;
    buffer_end = 0;
    special_start = 0;
    special_end = 0;
    kbd_state.shift_pressed = 0;
    kbd_state.ctrl_pressed = 0;
    kbd_state.alt_pressed = 0;
    kbd_state.caps_lock = 0;
}

void keyboard_handler_c(void) {
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);
    uint8_t key_released = scancode & 0x80;
    scancode &= 0x7F;
    
    if (scancode == KEY_LEFT_SHIFT || scancode == KEY_RIGHT_SHIFT) {
        kbd_state.shift_pressed = !key_released;
        return;
    }
    
    if (scancode == KEY_LEFT_CTRL) {
        kbd_state.ctrl_pressed = !key_released;
        return;
    }
    
    if (scancode == KEY_LEFT_ALT) {
        kbd_state.alt_pressed = !key_released;
        return;
    }
    
    if (scancode == KEY_CAPS_LOCK && !key_released) {
        kbd_state.caps_lock = !kbd_state.caps_lock;
        return;
    }
    
    if (!key_released) {
        if (scancode == KEY_ARROW_UP || scancode == KEY_ARROW_DOWN ||
            scancode == KEY_ARROW_LEFT || scancode == KEY_ARROW_RIGHT ||
            scancode == KEY_HOME || scancode == KEY_END || scancode == KEY_DELETE) {
            
            int next = (special_end + 1) % BUFFER_SIZE;
            if (next != special_start) {
                special_key_buffer[special_end] = scancode;
                special_end = next;
            }
        } else if (scancode < 128) {
            char c = 0;
            uint8_t use_upper = kbd_state.shift_pressed ^ kbd_state.caps_lock;
            
            if (use_upper) {
                c = scancode_to_ascii_upper[scancode];
            } else {
                c = scancode_to_ascii_lower[scancode];
            }
            
            if (c != 0) {
                int next = (buffer_end + 1) % BUFFER_SIZE;
                if (next != buffer_start) {
                    key_buffer[buffer_end] = c;
                    buffer_end = next;
                }
            }
        }
    }
}

int keyboard_has_key(void) {
    return buffer_start != buffer_end;
}

char keyboard_getchar(void) {
    if (buffer_start == buffer_end) {
        return 0;
    }
    
    char c = key_buffer[buffer_start];
    buffer_start = (buffer_start + 1) % BUFFER_SIZE;
    return c;
}

int keyboard_has_special_key(void) {
    return special_start != special_end;
}

uint8_t keyboard_get_special_key(void) {
    if (special_start == special_end) {
        return 0;
    }
    
    uint8_t key = special_key_buffer[special_start];
    special_start = (special_start + 1) % BUFFER_SIZE;
    return key;
}

keyboard_state_t keyboard_get_state(void) {
    return kbd_state;
}