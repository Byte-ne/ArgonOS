; idt_asm.asm - IDT loading and ISR stubs

section .text

; External C handlers
extern isr_handler
extern irq_handler

; Load IDT
global idt_load
idt_load:
    mov eax, [esp + 4]    ; Get IDT pointer from parameter
    lidt [eax]            ; Load IDT
    ret

; Common ISR stub (for exceptions 0-31)
isr_common_stub:
    pusha                  ; Save all registers
    
    push esp               ; Push stack pointer (not used for ISRs yet)
    call isr_handler       ; Call C handler
    add esp, 4             ; Clean up stack
    
    popa                   ; Restore registers
    add esp, 8             ; Clean up error code and interrupt number
    iret                   ; Return from interrupt

; Common IRQ stub (for IRQs 0-15, mapped to 32-47)
irq_common_stub:
    pusha                  ; Save all registers: EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI
    
    mov eax, esp           ; EAX = pointer to registers structure
    push eax               ; Pass registers pointer as 2nd parameter
    
    mov eax, [esp + 36]    ; Get interrupt number from stack (after pusha + push)
    push eax               ; Pass interrupt number as 1st parameter
    
    call irq_handler       ; Call C handler
    
    add esp, 8             ; Clean up parameters
    popa                   ; Restore all registers
    add esp, 8             ; Clean up error code and interrupt number
    iret                   ; Return from interrupt

; ISR stubs (0-31) - CPU exceptions
%macro ISR_NOERRCODE 1
global isr%1
isr%1:
    push 0                 ; Push dummy error code
    push %1                ; Push interrupt number
    jmp isr_common_stub
%endmacro

%macro ISR_ERRCODE 1
global isr%1
isr%1:
    push %1                ; Push interrupt number
    jmp isr_common_stub
%endmacro

; Define exception ISRs
ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE   8
ISR_NOERRCODE 9
ISR_ERRCODE   10
ISR_ERRCODE   11
ISR_ERRCODE   12
ISR_ERRCODE   13
ISR_ERRCODE   14
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_NOERRCODE 17
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_NOERRCODE 30
ISR_NOERRCODE 31

; IRQ stubs (32-47)
%macro IRQ 2
global irq%1
irq%1:
    push 0                 ; Push dummy error code
    push %2                ; Push interrupt number
    jmp irq_common_stub
%endmacro

IRQ 0, 32     ; Timer
IRQ 1, 33     ; Keyboard
IRQ 2, 34
IRQ 3, 35
IRQ 4, 36
IRQ 5, 37
IRQ 6, 38
IRQ 7, 39
IRQ 8, 40
IRQ 9, 41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44
IRQ 13, 45
IRQ 14, 46
IRQ 15, 47