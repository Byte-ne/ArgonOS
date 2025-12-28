; Multiboot header constants
MBALIGN  equ  1 << 0
MEMINFO  equ  1 << 1
FLAGS    equ  MBALIGN | MEMINFO
MAGIC    equ  0x1BADB002
CHECKSUM equ -(MAGIC + FLAGS)

; Multiboot header
section .multiboot
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

; Stack
section .bss
align 16
stack_bottom:
resb 16384 ; 16 KB stack
stack_top:

; Entry point
section .text
global _start
extern kernel_main

_start:
    mov esp, stack_top
    
    ; Call kernel main
    call kernel_main
    
    ; Hang if kernel returns
    cli
.hang:
    hlt
    jmp .hang