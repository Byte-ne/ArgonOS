#ifndef COMMAND_H
#define COMMAND_H

#include "types.h"

#define MAX_ARGS 10
#define MAX_FLAGS 5

typedef struct {
    char category[32];
    char command[32];
    char args[MAX_ARGS][64];
    int arg_count;
    char flags[MAX_FLAGS][32];
    char flag_values[MAX_FLAGS][64];
    int flag_count;
} parsed_command_t;

typedef void (*command_handler_t)(parsed_command_t* cmd);

typedef struct {
    char category[32];
    char command[32];
    command_handler_t handler;
    char description[128];
} command_entry_t;

void command_init(void);
void command_register(const char* category, const char* command, command_handler_t handler, const char* description);
int command_parse(const char* input, parsed_command_t* cmd);
void command_execute(const char* input);
char* command_get_flag_value(parsed_command_t* cmd, const char* flag);

#endif