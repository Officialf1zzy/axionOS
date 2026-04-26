#ifndef IO_H
#define IO_H

#include <stdint.h>

/**
 * io.h — x86 port I/O primitives
 * Thin wrappers around IN/OUT instructions.
 */

/* Read a byte from an I/O port */
static inline uint8_t inb(uint16_t port) {
    uint8_t val;
    __asm__ volatile ("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

/* Write a byte to an I/O port */
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

/* Small I/O delay (one dummy port write) — needed on older hardware */
static inline void io_wait(void) {
    outb(0x80, 0x00);
}

#endif /* IO_H */
