; task_switch.asm - Context switching support
; This file provides assembly support for saving/restoring task state

section .text

; External scheduler function
extern schedule

; Called by timer interrupt to switch tasks
global schedule_wrapper
schedule_wrapper:
    ; Save all registers on stack (creating registers_t structure)
    pusha           ; Push: EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI
    
    ; Push current ESP (pointer to saved registers)
    mov eax, esp
    push eax
    
    ; Call C scheduler function
    call schedule
    
    ; Clean up parameter
    add esp, 4
    
    ; Restore all registers (potentially from different task)
    popa            ; Pop: EDI, ESI, EBP, ESP, EBX, EDX, ECX, EAX
    
    ; Return from interrupt (will use potentially different EIP)
    iret