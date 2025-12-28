#ifndef LINEEDIT_H
#define LINEEDIT_H

#include "types.h"

#define LINE_BUFFER_SIZE 256
#define HISTORY_SIZE 20
#define CLIPBOARD_SIZE 256

void lineedit_init(void);
void lineedit_handle_key(char key);
void lineedit_handle_special_key(uint8_t scancode);
void lineedit_handle_ctrl_key(char key);
char* lineedit_get_line(void);
int lineedit_is_complete(void);
void lineedit_reset(void);
void lineedit_set_prompt(const char* prompt);
int lineedit_get_cursor_column(void);

#endif