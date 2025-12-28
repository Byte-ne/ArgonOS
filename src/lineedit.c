#include "lineedit.h"
#include "string.h"
#include "kernel.h"
#include "vga_cursor.h"

static char line_buffer[LINE_BUFFER_SIZE];
static int cursor_pos = 0;
static int line_length = 0;
static int is_complete = 0;

static char history[HISTORY_SIZE][LINE_BUFFER_SIZE];
static int history_count = 0;
static int history_index = -1;
static char temp_buffer[LINE_BUFFER_SIZE];

static char clipboard[CLIPBOARD_SIZE];

static char prompt[64] = "C:\\Argon> ";
static int prompt_len = 10;

void lineedit_init(void) {
    memset(line_buffer, 0, LINE_BUFFER_SIZE);
    memset(clipboard, 0, CLIPBOARD_SIZE);
    cursor_pos = 0;
    line_length = 0;
    is_complete = 0;
    history_count = 0;
    history_index = -1;
}

void lineedit_set_prompt(const char* new_prompt) {
    strcpy(prompt, new_prompt);
    prompt_len = strlen(prompt);
}

int lineedit_get_cursor_column(void) {
    return prompt_len + cursor_pos;
}

static void save_to_history(const char* line) {
    if (strlen(line) == 0) return;
    
    if (history_count > 0 && strcmp(history[history_count - 1], line) == 0) {
        return;
    }
    
    if (history_count < HISTORY_SIZE) {
        strcpy(history[history_count], line);
        history_count++;
    } else {
        for (int i = 0; i < HISTORY_SIZE - 1; i++) {
            strcpy(history[i], history[i + 1]);
        }
        strcpy(history[HISTORY_SIZE - 1], line);
    }
}

static void update_cursor(void) {
    vga_cursor_set_position(prompt_len + cursor_pos, terminal_get_row());
}

static void redraw_line(void) {
    terminal_putchar('\r');
    
    for (int i = 0; i < prompt_len + line_length + 5; i++) {
        terminal_putchar(' ');
    }
    
    terminal_putchar('\r');
    terminal_writestring(prompt);
    terminal_writestring(line_buffer);
    
    int final_pos = prompt_len + cursor_pos;
    int current_pos = prompt_len + line_length;
    
    while (current_pos > final_pos) {
        terminal_putchar('\b');
        current_pos--;
    }
    
    update_cursor();
}

static void handle_arrow_up(void) {
    if (history_count == 0) return;
    
    if (history_index == -1) {
        strcpy(temp_buffer, line_buffer);
        history_index = history_count - 1;
    } else if (history_index > 0) {
        history_index--;
    } else {
        return;
    }
    
    strcpy(line_buffer, history[history_index]);
    line_length = strlen(line_buffer);
    cursor_pos = line_length;
    redraw_line();
}

static void handle_arrow_down(void) {
    if (history_index == -1) return;
    
    if (history_index < history_count - 1) {
        history_index++;
        strcpy(line_buffer, history[history_index]);
    } else {
        history_index = -1;
        strcpy(line_buffer, temp_buffer);
    }
    
    line_length = strlen(line_buffer);
    cursor_pos = line_length;
    redraw_line();
}

static void handle_arrow_left(void) {
    if (cursor_pos > 0) {
        cursor_pos--;
        terminal_putchar('\b');
        update_cursor();
    }
}

static void handle_arrow_right(void) {
    if (cursor_pos < line_length) {
        terminal_putchar(line_buffer[cursor_pos]);
        cursor_pos++;
        update_cursor();
    }
}

static void handle_home(void) {
    while (cursor_pos > 0) {
        terminal_putchar('\b');
        cursor_pos--;
    }
    update_cursor();
}

static void handle_end(void) {
    while (cursor_pos < line_length) {
        terminal_putchar(line_buffer[cursor_pos]);
        cursor_pos++;
    }
    update_cursor();
}

static void handle_delete(void) {
    if (cursor_pos < line_length) {
        for (int i = cursor_pos; i < line_length - 1; i++) {
            line_buffer[i] = line_buffer[i + 1];
        }
        line_length--;
        line_buffer[line_length] = '\0';
        redraw_line();
    }
}

void lineedit_handle_special_key(uint8_t scancode) {
    if (scancode == 0x48) {
        handle_arrow_up();
    } else if (scancode == 0x50) {
        handle_arrow_down();
    } else if (scancode == 0x4B) {
        handle_arrow_left();
    } else if (scancode == 0x4D) {
        handle_arrow_right();
    } else if (scancode == 0x47) {
        handle_home();
    } else if (scancode == 0x4F) {
        handle_end();
    } else if (scancode == 0x53) {
        handle_delete();
    }
}

void lineedit_handle_ctrl_key(char key) {
    if (key == 'c') {
        if (line_length > 0) {
            strcpy(clipboard, line_buffer);
            terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN, VGA_BLACK));
            terminal_writestring(" [Copied]");
            terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
            update_cursor();
        }
    } else if (key == 'v') {
        if (strlen(clipboard) > 0) {
            for (int i = 0; clipboard[i] != '\0' && line_length < LINE_BUFFER_SIZE - 1; i++) {
                if (cursor_pos < line_length) {
                    for (int j = line_length; j > cursor_pos; j--) {
                        line_buffer[j] = line_buffer[j - 1];
                    }
                }
                
                line_buffer[cursor_pos] = clipboard[i];
                cursor_pos++;
                line_length++;
            }
            
            line_buffer[line_length] = '\0';
            redraw_line();
        }
    } else if (key == 'l') {
        terminal_clear();
        terminal_writestring(prompt);
        terminal_writestring(line_buffer);
        
        int final_pos = prompt_len + cursor_pos;
        int current_pos = prompt_len + line_length;
        
        while (current_pos > final_pos) {
            terminal_putchar('\b');
            current_pos--;
        }
        
        update_cursor();
    }
}

void lineedit_handle_key(char key) {
    if (key == '\n') {
        terminal_putchar('\n');
        line_buffer[line_length] = '\0';
        save_to_history(line_buffer);
        is_complete = 1;
        history_index = -1;
        return;
    }
    
    if (key == '\b') {
        if (cursor_pos > 0) {
            for (int i = cursor_pos - 1; i < line_length - 1; i++) {
                line_buffer[i] = line_buffer[i + 1];
            }
            line_length--;
            cursor_pos--;
            line_buffer[line_length] = '\0';
            redraw_line();
        }
        return;
    }
    
    if (key == '\t') {
        return;
    }
    
    if (line_length < LINE_BUFFER_SIZE - 1) {
        if (cursor_pos < line_length) {
            for (int i = line_length; i > cursor_pos; i--) {
                line_buffer[i] = line_buffer[i - 1];
            }
        }
        
        line_buffer[cursor_pos] = key;
        cursor_pos++;
        line_length++;
        line_buffer[line_length] = '\0';
        
        if (cursor_pos == line_length) {
            terminal_putchar(key);
            update_cursor();
        } else {
            redraw_line();
        }
    }
}

char* lineedit_get_line(void) {
    return line_buffer;
}

int lineedit_is_complete(void) {
    return is_complete;
}

void lineedit_reset(void) {
    memset(line_buffer, 0, LINE_BUFFER_SIZE);
    cursor_pos = 0;
    line_length = 0;
    is_complete = 0;
}