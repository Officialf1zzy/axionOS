#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

// Initialize the PIT timer (sets frequency to ~1000 Hz)
void timer_init(void);

// IRQ0 handler (called from your IDT/interrupt system)
void timer_irq_handler(void);

// Sleep for a number of milliseconds
void timer_sleep(uint32_t ms);

// Optional: get current tick count
uint32_t timer_get_ticks(void);

#endif
