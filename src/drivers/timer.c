#include <stdint.h>

/* send EOI to PIC */
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

#include "pic.h"

void timer_irq_handler(void) {
    pic_send_eoi(IRQ_TIMER);
}