#include <stdint.h>
#include "timer.h"

static volatile uint32_t tick = 0;

/* send EOI to PIC */
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

#include "pic.h"

void timer_irq_handler(void) {
    tick++;
    pic_send_eoi(IRQ_TIMER);
}

void timer_sleep(uint32_t ms) {
    uint32_t start = tick;
    while ((tick - start) < ms) {
        __asm__ volatile ("hlt");
    }
}

void timer_init() {
    uint32_t divisor = 1193180 / 1000;

    outb(0x43, 0x36);
    outb(0x40, divisor & 0xFF);
    outb(0x40, (divisor >> 8) & 0xFF);
}

uint32_t timer_get_ticks(void) {
    return tick;
}
