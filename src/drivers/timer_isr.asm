[BITS 32]
[SECTION .text]

GLOBAL timer_isr_stub
EXTERN timer_irq_handler

timer_isr_stub:
    pusha

    push ds
    push es
    push fs
    push gs

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov eax, esp
    and esp, 0xFFFFFFF0
    push eax

    call timer_irq_handler

    pop esp

    pop gs
    pop fs
    pop es
    pop ds

    popa
    iret