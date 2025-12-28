[BITS 32]

global task_switch_context
extern scheduler_switch_task

task_switch_context:
    pushad
    pushfd
    
    mov eax, esp
    push eax
    call scheduler_switch_task
    add esp, 4
    
    popfd
    popad
    ret