; paging_asm.asm - Assembly routines for paging

section .text

; Enable paging
; Parameter: Physical address of page directory (in stack)
global paging_enable
paging_enable:
    push ebp
    mov ebp, esp
    
    ; Get page directory address from parameter
    mov eax, [ebp + 8]
    
    ; Load page directory into CR3
    mov cr3, eax
    
    ; Enable paging by setting bit 31 of CR0
    mov eax, cr0
    or eax, 0x80000000      ; Set PG bit (bit 31)
    mov cr0, eax
    
    pop ebp
    ret