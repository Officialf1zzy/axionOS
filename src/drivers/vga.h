/**
 * vga.h — VGA Text Mode Driver for UEFI/Bare-Metal Kernels
 *
 * Targets the VGA text-mode framebuffer at physical address 0xB8000.
 * Works after UEFI hands control to the kernel (ExitBootServices).
 *
 * Usage in your kernel:
 *   #include "vga.h"
 *   vga_init();
 *   printc("Hello, kernel world!\n");
 */

#ifndef VGA_H
#define VGA_H
#include <stdint.h>
/* ── Dimensions ──────────────────────────────────────────────────────────── */
#define VGA_WIDTH   80
#define VGA_HEIGHT  25

/* ── Physical base address of the VGA text buffer ────────────────────────── */
#define VGA_BASE    ((volatile uint16_t *)0xB8000)

/* ── Attribute colours (foreground / background) ─────────────────────────── */
typedef enum {
    VGA_COLOR_BLACK         = 0,
    VGA_COLOR_BLUE          = 1,
    VGA_COLOR_GREEN         = 2,
    VGA_COLOR_CYAN          = 3,
    VGA_COLOR_RED           = 4,
    VGA_COLOR_MAGENTA       = 5,
    VGA_COLOR_BROWN         = 6,
    VGA_COLOR_LIGHT_GREY    = 7,
    VGA_COLOR_DARK_GREY     = 8,
    VGA_COLOR_LIGHT_BLUE    = 9,
    VGA_COLOR_LIGHT_GREEN   = 10,
    VGA_COLOR_LIGHT_CYAN    = 11,
    VGA_COLOR_LIGHT_RED     = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_YELLOW        = 14,
    VGA_COLOR_WHITE         = 15,
} vga_color_t;

/* ── Public API ──────────────────────────────────────────────────────────── */

/**
 * vga_init()
 *   Must be called once before any other VGA function.
 *   Clears the screen and resets the cursor to (0, 0).
 */
void vga_init(void);

/**
 * vga_set_color(fg, bg)
 *   Change the active foreground / background colour used by all
 *   subsequent print calls.
 */
void vga_set_color(vga_color_t fg, vga_color_t bg);

/**
 * vga_set_cursor(col, row)
 *   Move the hardware text cursor to the given column / row.
 *   Both values are zero-based.
 */
void vga_set_cursor(uint8_t col, uint8_t row);

/**
 * vga_clear()
 *   Fill the entire screen with spaces using the current colour and
 *   reset the software cursor to (0, 0).
 */
void vga_clear(void);

/**
 * vga_putchar(c)
 *   Write a single character at the current cursor position and advance
 *   the cursor.  Handles '\n', '\r', '\t', and '\b'.
 */
void vga_putchar(char c);

/**
 * printc(str)
 *   Write a null-terminated string to the VGA display.
 *   This is the function you call from your kernel code.
 */
void printc(const char *str);

/**
 * printc_at(str, col, row)
 *   Write a null-terminated string starting at an explicit (col, row)
 *   position without altering the normal cursor.
 */
void printc_at(const char *str, uint8_t col, uint8_t row);

/**
 * printc_hex(value)
 *   Print an unsigned 64-bit value as "0xHHHHHHHHHHHHHHHH".
 *   Useful for dumping addresses / register values from the kernel.
 */
void printc_hex(uint64_t value);

/**
 * printc_dec(value)
 *   Print an unsigned 64-bit value in decimal.
 */

#endif /* VGA_H */
