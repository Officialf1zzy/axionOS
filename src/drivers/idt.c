#include "idt.h"
#include <stddef.h>
#include <stdint.h>

#define IDT_ENTRIES 256

static idt_entry_t idt[IDT_ENTRIES];
static idt_ptr_t   idt_ptr;

/* external ISR stubs (from ASM) */
extern void timer_isr_stub();
extern void keyboard_isr_stub();

/* load IDT */
static inline void lidt(idt_ptr_t *ptr) {
    __asm__ volatile ("lidt (%0)" : : "r"(ptr));
}

void idt_set_gate(uint8_t vector, uint32_t handler,
                  uint16_t selector, uint8_t type_attr)
{
    idt[vector].offset_low  = handler & 0xFFFF;
    idt[vector].offset_high = (handler >> 16) & 0xFFFF;
    idt[vector].selector    = selector;
    idt[vector].zero        = 0;
    idt[vector].type_attr   = type_attr;
}

void idt_init(void)
{
    /* clear IDT */
    for (int i = 0; i < IDT_ENTRIES; i++) {
        idt[i] = (idt_entry_t){0};
    }

    /* IRQs (after PIC remap → 32–47) */
    idt_set_gate(32, (uint32_t)timer_isr_stub,    0x08, 0x8E);
    idt_set_gate(33, (uint32_t)keyboard_isr_stub, 0x08, 0x8E);

    /* IDT pointer */
    idt_ptr.limit = sizeof(idt) - 1;
    idt_ptr.base  = (uint32_t)&idt;

    lidt(&idt_ptr);
}
