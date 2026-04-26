; keyboard_isr.asm — IRQ1 (PS/2 keyboard) interrupt service routine stub
;
; NASM syntax, 32-bit protected mode.
;
; The CPU pushes FLAGS, CS, EIP automatically on INT.
; We must save/restore all general-purpose registers (PUSHA/POPA),
; call our C handler, then return with IRET.

[BITS 32]
[SECTION .text]

; The C handler we call after saving state
EXTERN keyboard_irq_handler

; Make our stub visible to C (referenced in idt.h / idt.c)
GLOBAL keyboard_isr_stub

keyboard_isr_stub:
    PUSHA

    PUSH DS
    PUSH ES
    PUSH FS
    PUSH GS

    MOV AX, 0x10
    MOV DS, AX
    MOV ES, AX
    MOV FS, AX
    MOV GS, AX

    MOV EAX, ESP
    AND ESP, 0xFFFFFFF0
    PUSH EAX

    CALL keyboard_irq_handler

    POP ESP

    POP GS
    POP FS
    POP ES
    POP DS

    POPA
    IRET
