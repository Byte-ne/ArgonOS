#include "command.h"
#include "string.h"
#include "kernel.h"

#define MAX_COMMANDS 50

static command_entry_t commands[MAX_COMMANDS];
static int command_count = 0;

void command_init(void) {
    command_count = 0;
}

void command_register(const char* category, const char* command, command_handler_t handler, const char* description) {
    if (command_count >= MAX_COMMANDS) return;
    
    strcpy(commands[command_count].category, category);
    strcpy(commands[command_count].command, command);
    commands[command_count].handler = handler;
    strcpy(commands[command_count].description, description);
    command_count++;
}

int command_parse(const char* input, parsed_command_t* cmd) {
    memset(cmd, 0, sizeof(parsed_command_t));
    
    char buffer[256];
    strcpy(buffer, input);
    
    char* dot = strstr(buffer, ".");
    if (!dot) return 0;
    
    *dot = '\0';
    strcpy(cmd->category, buffer);
    
    char* space = strstr(dot + 1, " ");
    if (space) {
        *space = '\0';
        strcpy(cmd->command, dot + 1);
        
        char* token = space + 1;
        while (*token == ' ') token++;
        
        while (*token) {
            if (strncmp(token, "--", 2) == 0) {
                token += 2;
                char* value_start = token;
                
                while (*value_start && *value_start != ' ' && *value_start != '=') {
                    value_start++;
                }
                
                int flag_len = (int)(value_start - token);
                strncpy(cmd->flags[cmd->flag_count], token, flag_len);
                cmd->flags[cmd->flag_count][flag_len] = '\0';
                
                if (*value_start == '=') {
                    value_start++;
                    char* value_end = value_start;
                    while (*value_end && *value_end != ' ') value_end++;
                    
                    int value_len = (int)(value_end - value_start);
                    strncpy(cmd->flag_values[cmd->flag_count], value_start, value_len);
                    cmd->flag_values[cmd->flag_count][value_len] = '\0';
                    
                    token = value_end;
                } else {
                    cmd->flag_values[cmd->flag_count][0] = '\0';
                    token = value_start;
                }
                
                cmd->flag_count++;
            } else {
                char* next_space = strstr(token, " ");
                int arg_len = next_space ? (int)(next_space - token) : (int)strlen(token);
                
                strncpy(cmd->args[cmd->arg_count], token, arg_len);
                cmd->args[cmd->arg_count][arg_len] = '\0';
                cmd->arg_count++;
                
                token = next_space ? next_space + 1 : token + arg_len;
            }
            
            while (*token == ' ') token++;
        }
    } else {
        strcpy(cmd->command, dot + 1);
    }
    
    return 1;
}

void command_execute(const char* input) {
    parsed_command_t cmd;
    
    if (!command_parse(input, &cmd)) {
        terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
        terminal_writestring("Invalid command format. Use: category.command\n");
        return;
    }
    
    for (int i = 0; i < command_count; i++) {
        if (strcmp(commands[i].category, cmd.category) == 0 &&
            strcmp(commands[i].command, cmd.command) == 0) {
            commands[i].handler(&cmd);
            return;
        }
    }
    
    terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
    terminal_writestring("'");
    terminal_writestring(cmd.category);
    terminal_writestring(".");
    terminal_writestring(cmd.command);
    terminal_writestring("' is not recognized as a valid command.\n");
}

char* command_get_flag_value(parsed_command_t* cmd, const char* flag) {
    for (int i = 0; i < cmd->flag_count; i++) {
        if (strcmp(cmd->flags[i], flag) == 0) {
            return cmd->flag_values[i];
        }
    }
    return NULL;
}