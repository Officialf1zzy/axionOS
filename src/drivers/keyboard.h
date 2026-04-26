#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>
#include <stdbool.h>

/**
 * keyboard.h — Bare-metal PS/2 keyboard driver
 *
 * Hardware overview
 * -----------------
 *  Port 0x60  : PS/2 data register   (read scancode / write command)
 *  Port 0x64  : PS/2 status register (read) / command register (write)
 *
 *  IRQ line   : IRQ1  (INT 33 after standard PIC remap)
 *  Scancode   : Set 1 (IBM XT set — the power-on default on virtually
 *               every PC-compatible keyboard controller)
 *
 * Scancode anatomy (Set 1)
 * ------------------------
 *  • Make code  : 0x01-0x58   — key pressed
 *  • Break code : make | 0x80 — key released  (bit 7 set)
 *  • Extended   : 0xE0 prefix — arrow keys, numpad, etc. (handled
 *                 with a simple two-byte state machine)
 *
 * Modifier state
 * --------------
 *  Shift (L/R), Caps Lock, Num Lock, and Scroll Lock are tracked so
 *  printable characters come out in the right case.
 *
 * Key buffer
 * ----------
 *  A small circular FIFO holds decoded ASCII characters until the
 *  kernel consumes them via keyboard_getchar().
 *
 * Special keys
 * ------------
 *  ENTER     → '\n'   (0x0A)
 *  BACKSPACE → '\b'   (0x08)
 *  TAB       → '\t'   (0x09)
 *  ESC       → 0x1B
 */

/* ---- PS/2 port addresses ---- */
#define PS2_DATA_PORT   0x60
#define PS2_STATUS_PORT 0x64
#define PS2_CMD_PORT    0x64    /* same physical port, different direction */

/* Status register bits */
#define PS2_STATUS_OBF  (1 << 0)    /* output buffer full — data ready to read */
#define PS2_STATUS_IBF  (1 << 1)    /* input buffer full  — controller busy    */

/* Keyboard controller commands */
#define PS2_CMD_DISABLE_PORT1   0xAD
#define PS2_CMD_ENABLE_PORT1    0xAE
#define PS2_CMD_READ_CONFIG     0x20
#define PS2_CMD_WRITE_CONFIG    0x60

/* Key buffer capacity (must be a power of 2) */
#define KB_BUFFER_SIZE  64

/* ---- Exported types ---- */

/* Modifier-key flags kept in keyboard_state_t */
#define KB_MOD_LSHIFT   (1 << 0)
#define KB_MOD_RSHIFT   (1 << 1)
#define KB_MOD_CTRL     (1 << 2)
#define KB_MOD_ALT      (1 << 3)
#define KB_MOD_CAPSLOCK (1 << 4)
#define KB_MOD_NUMLOCK  (1 << 5)
#define KB_MOD_SCRLOCK  (1 << 6)

typedef struct {
    uint8_t  modifiers;     /* bitmask of KB_MOD_* flags */
    bool     extended;      /* true while processing 0xE0 prefix */
    /* circular FIFO */
    char     buf[KB_BUFFER_SIZE];
    volatile uint8_t head;  /* consumer index */
    volatile uint8_t tail;  /* producer index */
} keyboard_state_t;

/* ---- Public API ---- */

/**
 * keyboard_init() — Set up the IDT gate and unmask IRQ1.
 *
 * Prerequisites: idt_init() and pic_remap() must have been called,
 * and interrupts must still be disabled (CLI) so there is no race
 * between gate installation and the first interrupt.
 *
 * After this call:
 *   1. The IDT vector for INT_KEYBOARD points at keyboard_isr_stub.
 *   2. IRQ1 is unmasked in the PIC.
 *   3. Call STI to start receiving interrupts.
 */
void keyboard_init(void);

/**
 * keyboard_irq_handler() — Called by keyboard_isr_stub on every IRQ1.
 *
 * Reads the raw scancode from port 0x60, translates it to ASCII,
 * pushes printable characters (and \n / \b) into the ring buffer,
 * then sends EOI to the PIC.
 *
 * Do NOT call this directly — it is invoked by the ISR stub.
 */
void keyboard_irq_handler(void);

/**
 * keyboard_getchar() — Non-blocking key dequeue.
 *
 * Returns the next ASCII character from the ring buffer, or 0 if the
 * buffer is empty.  Call this from your kernel's main loop or from a
 * blocking wrapper that spins on keyboard_available().
 */
char keyboard_getchar(void);

/**
 * keyboard_available() — Returns true if at least one character is waiting.
 */
bool keyboard_available(void);

/**
 * keyboard_getchar_blocking() — Spin until a key is available, then return it.
 *
 * Useful for simple shells / early boot prompts.
 * Requires interrupts to be enabled (STI).
 */
char keyboard_getchar_blocking(void);

/**
 * keyboard_get_modifiers() — Snapshot of the current modifier state.
 */
uint8_t keyboard_get_modifiers(void);

#endif /* KEYBOARD_H */
