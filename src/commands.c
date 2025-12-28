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

    terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
    terminal_writestring("\n ID  Name              State       CPU Time\n");
    terminal_writestring("---  ----------------  ----------  ---------\n");

    for (int i = 0; i < MAX_TASKS; i++)
    {
        if (tasks[i].id != 0 && tasks[i].state != TASK_DEAD)
        {
            char buf[20];

            terminal_writestring(" ");
            itoa(tasks[i].id, buf);
            terminal_writestring(buf);
            terminal_writestring("   ");

            terminal_writestring(tasks[i].name);
            for (int j = strlen(tasks[i].name); j < 18; j++)
            {
                terminal_putchar(' ');
            }

            const char *state_str = "UNKNOWN";
            if (tasks[i].state == TASK_READY)
                state_str = "READY";
            else if (tasks[i].state == TASK_RUNNING)
                state_str = "RUNNING";
            else if (tasks[i].state == TASK_BLOCKED)
                state_str = "BLOCKED";

            terminal_writestring(state_str);
            for (int j = strlen(state_str); j < 12; j++)
            {
                terminal_putchar(' ');
            }

            itoa(tasks[i].cpu_time, buf);
            terminal_writestring(buf);
            terminal_putchar('\n');
        }
    }
    terminal_putchar('\n');
}

void cmd_process_create(parsed_command_t *cmd)
{
    if (cmd->arg_count == 0)
    {
        terminal_writestring("\nUsage: process.create \n");
        terminal_writestring("Available: counter, spinner, clock\n\n");
        return;
    }

    uint32_t task_id = 0;

    if (strcmp(cmd->args[0], "counter") == 0)
    {
        task_id = task_create("Counter", task_counter);
    }
    else if (strcmp(cmd->args[0], "spinner") == 0)
    {
        task_id = task_create("Spinner", task_spinner);
    }
    else if (strcmp(cmd->args[0], "clock") == 0)
    {
        task_id = task_create("Clock", task_clock);
    }
    else
    {
        terminal_setcolor(vga_entry_color(VGA_RED, VGA_BLACK));
        terminal_writestring("\nUnknown task type: ");
        terminal_writestring(cmd->args[0]);
        terminal_writestring("\n\n");
        terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
        return;
    }

    if (task_id > 0)
    {
        char buf[20];
        terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN, VGA_BLACK));
        terminal_writestring("\nTask created: ");
        terminal_writestring(cmd->args[0]);
        terminal_writestring(" (ID: ");
        itoa(task_id, buf);
        terminal_writestring(buf);
        terminal_writestring(")\n\n");
        terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
    }
    else
    {
        terminal_setcolor(vga_entry_color(VGA_RED, VGA_BLACK));
        terminal_writestring("\nFailed to create task!\n\n");
        terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
    }
}

void cmd_process_kill(parsed_command_t *cmd)
{
    if (cmd->arg_count == 0)
    {
        terminal_writestring("\nUsage: process.kill \n\n");
        return;
    }

    uint32_t task_id = atoi(cmd->args[0]);

    if (task_id == 0)
    {
        terminal_setcolor(vga_entry_color(VGA_RED, VGA_BLACK));
        terminal_writestring("\nCannot kill kernel task!\n\n");
        terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
        return;
    }

    task_kill(task_id);

    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN, VGA_BLACK));
    terminal_writestring("\nTask ");
    char buf[20];
    itoa(task_id, buf);
    terminal_writestring(buf);
    terminal_writestring(" terminated.\n\n");
    terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
}

void cmd_process_info(parsed_command_t *cmd)
{
    (void)cmd;

    task_t *current = task_get_current();

    terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
    terminal_writestring("\nProcess Information:\n");
    terminal_writestring("  Current task: ");
    terminal_writestring(current->name);
    terminal_writestring(" (ID: ");
    char buf[20];
    itoa(current->id, buf);
    terminal_writestring(buf);
    terminal_writestring(")\n");

    terminal_writestring("  Total tasks: ");
    itoa(task_get_count(), buf);
    terminal_writestring(buf);
    terminal_writestring("\n");

    terminal_writestring("  Counter value: ");
    itoa(get_counter_value(), buf);
    terminal_writestring(buf);
    terminal_writestring("\n\n");
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
    terminal_writestring("DISPLAY COMMANDS:\n");
    terminal_writestring("  process.list           - List processes\n\n");
    terminal_writestring("  process.create <name>  - Create process\n\n");
    terminal_writestring("  process.kill <id>      - Kill N process\n\n");
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