#ifndef PIC_H
#define PIC_H

#include <stdint.h>

/**
 * pic.h — 8259A Programmable Interrupt Controller
 *
 * The PC has two cascaded PICs (master + slave).
 * By default the master maps IRQ0-7 to INT 0x08-0x0F, which overlaps
 * CPU exception vectors.  We remap them to 0x20-0x27 (master) and
 * 0x28-0x2F (slave) so IRQ1 (keyboard) lands at vector 0x21 (33).
 */

#define PIC1_CMD    0x20    /* master command port   */
#define PIC1_DATA   0x21    /* master data/mask port */
#define PIC2_CMD    0xA0    /* slave  command port   */
#define PIC2_DATA   0xA1    /* slave  data/mask port */

#define PIC_EOI     0x20    /* End-Of-Interrupt command */

/* Vector offsets after remapping */
#define PIC1_OFFSET 0x20    /* IRQ 0-7  → INT 32-39  */
#define PIC2_OFFSET 0x28    /* IRQ 8-15 → INT 40-47  */

/* IRQ line for the PS/2 keyboard */
#define IRQ_KEYBOARD 1
#define INT_KEYBOARD (PIC1_OFFSET + IRQ_KEYBOARD)   /* = 33 = 0x21 */

#define IRQ_TIMER     0
#define IRQ_KEYBOARD  1

/**
 * pic_remap() — Remap both PICs to avoid clashing with CPU exceptions.
 * Must be called before enabling interrupts.
 */
void pic_remap(void);

/**
 * pic_send_eoi() — Signal end-of-interrupt for a given IRQ line.
 * Must be called at the end of every IRQ handler.
 */
void pic_send_eoi(uint8_t irq);

/**
 * pic_set_mask() / pic_clear_mask() — Enable/disable individual IRQ lines.
 */
void pic_set_mask(uint8_t irq);
void pic_clear_mask(uint8_t irq);

#endif /* PIC_H */
