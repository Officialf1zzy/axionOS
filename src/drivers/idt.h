#ifndef IDT_H
#define IDT_H

#include <stdint.h>

/**
 * idt.h — x86 Interrupt Descriptor Table
 *
 * We only need to register a handler for IRQ1 (keyboard, INT 33),
 * but a real kernel would fill all 256 entries.
 */

/* ---- IDT gate descriptor (8 bytes) ---- */
typedef struct __attribute__((packed)) {
    uint16_t offset_low;    /* bits  0-15 of handler address */
    uint16_t selector;      /* code segment selector (GDT)  */
    uint8_t  zero;          /* always 0                     */
    uint8_t  type_attr;     /* type + DPL + present bit     */
    uint16_t offset_high;   /* bits 16-31 of handler address */
} idt_entry_t;

/* ---- IDT pointer (used by LIDT instruction) ---- */
typedef struct __attribute__((packed)) {
    uint16_t limit;         /* sizeof(idt) - 1 */
    uint32_t base;          /* linear address of table */
} idt_ptr_t;

/* Gate type flags */
#define IDT_TYPE_INTERRUPT_32   0x8E    /* present, DPL=0, 32-bit interrupt gate */
#define IDT_TYPE_TRAP_32        0x8F    /* present, DPL=0, 32-bit trap gate      */

/**
 * idt_init() — Zero the IDT and load it with LIDT.
 */
void idt_init(void);

/**
 * idt_set_gate() — Install a handler for a given vector.
 * @vector    : interrupt vector number (0-255)
 * @handler   : linear address of the ISR stub
 * @selector  : code segment (0x08 in a flat 32-bit GDT)
 * @type_attr : gate flags (use IDT_TYPE_INTERRUPT_32)
 */
void idt_set_gate(uint8_t vector, uint32_t handler,
                  uint16_t selector, uint8_t type_attr);

/* ASM stubs declared in keyboard_isr.asm */
extern void keyboard_isr_stub(void);

#endif /* IDT_H */
