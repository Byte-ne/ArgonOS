#include "command.h"
#include "kernel.h"
#include "timer.h"
#include "memory.h"
#include "string.h"
#include "keyboard.h"
#include "task.h"
#include "scheduler.h"

extern void task_counter(void);
extern void task_spinner(void);
extern void task_clock(void);
extern uint32_t get_counter_value(void);
extern task_t tasks[];

void cmd_system_uptime(parsed_command_t *cmd)
{
    (void)cmd;

    uint32_t seconds = timer_get_seconds();
    uint32_t minutes = seconds / 60;
    uint32_t hours = minutes / 60;

    seconds %= 60;
    minutes %= 60;

    char buf[20];
    terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
    terminal_writestring("\nSystem uptime: ");

    if (hours > 0)
    {
        itoa(hours, buf);
        terminal_writestring(buf);
        terminal_writestring("h ");
    }

    itoa(minutes, buf);
    terminal_writestring(buf);
    terminal_writestring("m ");

    itoa(seconds, buf);
    terminal_writestring(buf);
    terminal_writestring("s\n\n");
}

void cmd_system_sleep(parsed_command_t *cmd)
{
    if (cmd->arg_count == 0)
    {
        terminal_writestring("\nUsage: system.sleep <seconds>\n\n");
        return;
    }

    int seconds = atoi(cmd->args[0]);
    if (seconds <= 0 || seconds > 60)
    {
        terminal_writestring("\nInvalid duration. Use 1-60 seconds.\n\n");
        return;
    }

    char *progress = command_get_flag_value(cmd, "progress");

    terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
    terminal_writestring("\nSleeping");

    for (int i = 0; i < seconds; i++)
    {
        timer_wait(100);
        if (progress)
        {
            terminal_putchar('.');
        }
    }

    terminal_writestring(" Done!\n\n");
}

void cmd_system_info(parsed_command_t *cmd)
{
    (void)cmd;

    terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
    terminal_writestring("\nArgon OS System Information:\n");
    terminal_writestring("  Version: 0.2.0\n");
    terminal_writestring("  Phase: 2.3A Complete\n");

    char buf[20];
    itoa(timer_get_seconds(), buf);
    terminal_writestring("  Uptime: ");
    terminal_writestring(buf);
    terminal_writestring(" seconds\n\n");
}

void cmd_system_keyboard(parsed_command_t *cmd)
{
    (void)cmd;

    keyboard_state_t state = keyboard_get_state();

    terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
    terminal_writestring("\nKeyboard Status:\n");

    terminal_writestring("  Shift:     ");
    terminal_writestring(state.shift_pressed ? "PRESSED\n" : "Released\n");

    terminal_writestring("  Ctrl:      ");
    terminal_writestring(state.ctrl_pressed ? "PRESSED\n" : "Released\n");

    terminal_writestring("  Alt:       ");
    terminal_writestring(state.alt_pressed ? "PRESSED\n" : "Released\n");

    terminal_writestring("  Caps Lock: ");
    terminal_writestring(state.caps_lock ? "ON\n" : "OFF\n");

    terminal_writestring("\n");
}

void cmd_memory_info(parsed_command_t *cmd)
{
    (void)cmd;

    char buf[20];

    terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
    terminal_writestring("\nMemory Information:\n");

    itoa(memory_get_total(), buf);
    terminal_writestring("  Total:  ");
    terminal_writestring(buf);
    terminal_writestring(" bytes\n");

    itoa(memory_get_used(), buf);
    terminal_writestring("  Used:   ");
    terminal_writestring(buf);
    terminal_writestring(" bytes\n");

    itoa(memory_get_free(), buf);
    terminal_writestring("  Free:   ");
    terminal_writestring(buf);
    terminal_writestring(" bytes\n\n");
}

void cmd_memory_alloc(parsed_command_t *cmd)
{
    if (cmd->arg_count == 0)
    {
        terminal_writestring("\nUsage: memory.alloc <size>\n");
        terminal_writestring("Example: memory.alloc 1024\n\n");
        return;
    }

    uint32_t size = atoi(cmd->args[0]);
    void *ptr = kmalloc(size);

    if (ptr)
    {
        char buf[20];
        terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN, VGA_BLACK));
        terminal_writestring("\nAllocated ");
        itoa(size, buf);
        terminal_writestring(buf);
        terminal_writestring(" bytes at address 0x");

        itoa((uint32_t)ptr, buf);
        terminal_writestring(buf);
        terminal_writestring("\n\n");
    }
    else
    {
        terminal_setcolor(vga_entry_color(VGA_RED, VGA_BLACK));
        terminal_writestring("\nMemory allocation failed!\n\n");
    }
}

void cmd_memory_test(parsed_command_t *cmd)
{
    (void)cmd;

    terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
    terminal_writestring("\nRunning memory test...\n");

    void *ptr1 = kmalloc(256);
    void *ptr2 = kmalloc(512);
    void *ptr3 = kmalloc(1024);

    if (ptr1 && ptr2 && ptr3)
    {
        terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN, VGA_BLACK));
        terminal_writestring("  [OK] Allocated 3 blocks\n");

        kfree(ptr2);
        terminal_writestring("  [OK] Freed middle block\n");

        void *ptr4 = kmalloc(512);
        if (ptr4)
        {
            terminal_writestring("  [OK] Reused freed space\n");
            kfree(ptr4);
        }

        kfree(ptr1);
        kfree(ptr3);
        terminal_writestring("  [OK] All memory freed\n\n");
    }
    else
    {
        terminal_setcolor(vga_entry_color(VGA_RED, VGA_BLACK));
        terminal_writestring("  [FAIL] Memory allocation failed\n\n");
    }
}

void cmd_display_clear(parsed_command_t *cmd)
{
    (void)cmd;
    terminal_clear();
}

void cmd_display_echo(parsed_command_t *cmd)
{
    char *color = command_get_flag_value(cmd, "color");

    if (color)
    {
        if (strcmp(color, "red") == 0)
            terminal_setcolor(vga_entry_color(VGA_RED, VGA_BLACK));
        else if (strcmp(color, "green") == 0)
            terminal_setcolor(vga_entry_color(VGA_GREEN, VGA_BLACK));
        else if (strcmp(color, "cyan") == 0)
            terminal_setcolor(vga_entry_color(VGA_CYAN, VGA_BLACK));
        else if (strcmp(color, "yellow") == 0)
            terminal_setcolor(vga_entry_color(VGA_YELLOW, VGA_BLACK));
        else
            terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
    }
    else
    {
        terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
    }

    terminal_putchar('\n');
    for (int i = 0; i < cmd->arg_count; i++)
    {
        terminal_writestring(cmd->args[i]);
        if (i < cmd->arg_count - 1)
            terminal_putchar(' ');
    }
    terminal_writestring("\n\n");
}

void cmd_process_list(parsed_command_t *cmd)
{
    (void)cmd;
    terminal_writestring("Active Tasks:\n");
    terminal_writestring("ID   Name                State\n");
    terminal_writestring("---  ------------------  -------\n");

    for (uint32_t i = 0; i < MAX_TASKS; i++)
    {
        task_t *task = task_get_by_index(i);
        if (task && task->id != 0 && task->state != TASK_DEAD)
        {
            char id_str[12];
            itoa(task->id, id_str);

            terminal_writestring(id_str);
            terminal_writestring("    ");
            terminal_writestring(task->name);

            // Pad name
            uint32_t name_len = strlen(task->name);
            for (uint32_t j = name_len; j < 20; j++)
            {
                terminal_putchar(' ');
            }

            if (task->state == TASK_RUNNING)
            {
                terminal_writestring("RUNNING\n");
            }
            else if (task->state == TASK_SLEEPING)
            {
                terminal_writestring("SLEEPING\n");
            }
        }
    }

    char count_str[12];
    itoa(task_count_active(), count_str);
    terminal_writestring("\nTotal active tasks: ");
    terminal_writestring(count_str);
    terminal_putchar('\n');
}

void cmd_process_create(parsed_command_t *cmd)
{
    const char *task_name = cmd->args[0];
    ;
    if (!task_name || strlen(task_name) == 0)
    {
        terminal_writestring("Usage: process.create <name>\n");
        terminal_writestring("Available tasks: counter, clock, spinner\n");
        return;
    }

    void (*entry_point)(void) = 0;

    if (strcmp(task_name, "counter") == 0)
    {
        entry_point = task_counter;
    }
    else if (strcmp(task_name, "clock") == 0)
    {
        entry_point = task_clock;
    }
    else if (strcmp(task_name, "spinner") == 0)
    {
        entry_point = task_spinner;
    }
    else
    {
        terminal_writestring("Unknown task type: ");
        terminal_writestring(task_name);
        terminal_putchar('\n');
        terminal_writestring("Available: counter, clock, spinner\n");
        return;
    }

    uint32_t task_id = task_create(task_name, entry_point);
    if (task_id == 0)
    {
        terminal_writestring("Failed to create task (max tasks reached)\n");
    }
    else
    {
        terminal_writestring("Created task '");
        terminal_writestring(task_name);
        terminal_writestring("' with ID ");
        char id_str[12];
        itoa(task_id, id_str);
        terminal_writestring(id_str);
        terminal_putchar('\n');
    }
}

void cmd_process_kill(parsed_command_t *cmd)
{
    const char *id_str = cmd->args[0];
    ;
    if (!id_str || strlen(id_str) == 0)
    {
        terminal_writestring("Usage: process.kill <task_id>\n");
        return;
    }

    uint32_t task_id = atoi(id_str);
    if (task_id == 0)
    {
        terminal_writestring("Cannot kill kernel task\n");
        return;
    }

    task_t *task = task_get_by_id(task_id);
    if (!task)
    {
        terminal_writestring("Task not found: ");
        terminal_writestring(id_str);
        terminal_putchar('\n');
        return;
    }

    task_kill(task_id);
    terminal_writestring("Killed task ");
    terminal_writestring(id_str);
    terminal_putchar('\n');
}

void cmd_process_info(parsed_command_t *cmd)
{
    const char *id_str = cmd->args[0];
    if (!id_str || strlen(id_str) == 0)
    {
        terminal_writestring("Usage: process.info <task_id>\n");
        return;
    }

    uint32_t task_id = atoi(id_str);
    task_t *task = task_get_by_id(task_id);

    if (!task)
    {
        terminal_writestring("Task not found\n");
        return;
    }

    terminal_writestring("Task Information:\n");
    terminal_writestring("  ID:    ");
    char id_buf[12];
    itoa(task->id, id_buf);
    terminal_writestring(id_buf);
    terminal_putchar('\n');

    terminal_writestring("  Name:  ");
    terminal_writestring(task->name);
    terminal_putchar('\n');

    terminal_writestring("  State: ");
    if (task->state == TASK_RUNNING)
    {
        terminal_writestring("RUNNING\n");
    }
    else if (task->state == TASK_SLEEPING)
    {
        terminal_writestring("SLEEPING\n");
    }
    else
    {
        terminal_writestring("DEAD\n");
    }
}

void cmd_help(parsed_command_t *cmd)
{
    (void)cmd;

    terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
    terminal_writestring("\nArgon OS Command Reference:\n\n");
    terminal_writestring("SYSTEM COMMANDS:\n");
    terminal_writestring("  system.uptime          - Show system uptime\n");
    terminal_writestring("  system.sleep <sec>     - Sleep for N seconds\n");
    terminal_writestring("  system.info            - Show system information\n");
    terminal_writestring("  system.keyboard        - Show keyboard status\n\n");
    terminal_writestring("MEMORY COMMANDS:\n");
    terminal_writestring("  memory.info            - Show memory statistics\n");
    terminal_writestring("  memory.alloc <size>    - Allocate memory\n");
    terminal_writestring("  memory.test            - Run memory test\n\n");
    terminal_writestring("DISPLAY COMMANDS:\n");
    terminal_writestring("  display.clear          - Clear the screen\n");
    terminal_writestring("  display.echo <text>    - Display text\n");
    terminal_writestring("  help.commands          - Show this help\n\n");
    terminal_writestring("PROCESS COMMANDS:\n");
    terminal_writestring("  process.list           - List processes\n");
    terminal_writestring("  process.create <name>  - Create process\n");
    terminal_writestring("  process.kill <id>      - Kill N process\n");
    terminal_writestring("  process.info           - Process Information\n\n");
}

void commands_register_all(void)
{
    command_register("system", "uptime", cmd_system_uptime, "Show system uptime");
    command_register("system", "sleep", cmd_system_sleep, "Sleep for N seconds");
    command_register("system", "info", cmd_system_info, "Show system info");
    command_register("system", "keyboard", cmd_system_keyboard, "Show keyboard status");

    command_register("memory", "info", cmd_memory_info, "Show memory stats");
    command_register("memory", "alloc", cmd_memory_alloc, "Allocate memory");
    command_register("memory", "test", cmd_memory_test, "Test memory system");

    command_register("display", "clear", cmd_display_clear, "Clear screen");
    command_register("display", "echo", cmd_display_echo, "Display text");

    command_register("process", "list", cmd_process_list, "List all processes");
    command_register("process", "create", cmd_process_create, "Create new process");
    command_register("process", "kill", cmd_process_kill, "Kill a process");
    command_register("process", "info", cmd_process_info, "Process information");

    command_register("help", "commands", cmd_help, "Show help");
}