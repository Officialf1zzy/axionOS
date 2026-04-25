/**
 * vga.c — VGA Text Mode Driver Implementation
 *
 * How the VGA text buffer works
 * ─────────────────────────────
 * The buffer lives at physical address 0xB8000.
 * It is a flat array of 80 × 25 = 2 000 uint16_t entries.
 *
 *   Entry layout (16 bits):
 *     [15:12]  background colour (4 bits)
 *     [11: 8]  foreground colour (4 bits)
 *     [ 7: 0]  ASCII character  (8 bits)
 *
 * The hardware text cursor is moved by writing to two VGA I/O ports:
 *   0x3D4  – index register
 *   0x3D5  – data  register
 *
 * Compile (freestanding, no stdlib):
 *   gcc -ffreestanding -nostdlib -c vga.c -o vga.o
 */

#include "vga.h"

/* ── I/O port helpers (inline asm, x86/x86-64) ───────────────────────────── */

static inline void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

/* ── VGA hardware registers ──────────────────────────────────────────────── */
#define VGA_CTRL_REG   0x3D4   /* CRT controller index  */
#define VGA_DATA_REG   0x3D5   /* CRT controller data   */
#define VGA_CURSOR_HI  0x0E    /* Cursor location high  */
#define VGA_CURSOR_LO  0x0F    /* Cursor location low   */

/* ── Driver state ────────────────────────────────────────────────────────── */
static volatile uint16_t *vga_buf;   /* pointer into 0xB8000      */
static uint8_t  vga_col;             /* current column (0-79)     */
static uint8_t  vga_row;             /* current row    (0-24)     */
static uint8_t  vga_attr;            /* packed colour attribute   */

/* ── Internal helpers ────────────────────────────────────────────────────── */

/** Pack foreground + background into the high byte of a VGA cell. */
static inline uint8_t make_attr(vga_color_t fg, vga_color_t bg)
{
    return (uint8_t)((bg << 4) | (fg & 0x0F));
}

/** Build a full 16-bit VGA cell from a character and attribute byte. */
static inline uint16_t make_entry(char c, uint8_t attr)
{
    return (uint16_t)((uint16_t)attr << 8) | (uint8_t)c;
}

/** Move the blinking hardware cursor to match the software cursor. */
static void update_hw_cursor(void)
{
    uint16_t pos = (uint16_t)(vga_row * VGA_WIDTH + vga_col);
    outb(VGA_CTRL_REG, VGA_CURSOR_HI);
    outb(VGA_DATA_REG, (uint8_t)(pos >> 8));
    outb(VGA_CTRL_REG, VGA_CURSOR_LO);
    outb(VGA_DATA_REG, (uint8_t)(pos & 0xFF));
}

/** Scroll the screen up by one line and blank the last row. */
static void scroll_up(void)
{
    /* Move every row one line up. */
    for (uint8_t row = 1; row < VGA_HEIGHT; row++) {
        for (uint8_t col = 0; col < VGA_WIDTH; col++) {
            uint16_t src  = (uint16_t)( row      * VGA_WIDTH + col);
            uint16_t dst  = (uint16_t)((row - 1) * VGA_WIDTH + col);
            vga_buf[dst] = vga_buf[src];
        }
    }

    /* Blank the last row. */
    uint16_t blank = make_entry(' ', vga_attr);
    for (uint8_t col = 0; col < VGA_WIDTH; col++) {
        vga_buf[(VGA_HEIGHT - 1) * VGA_WIDTH + col] = blank;
    }

    /* Keep the cursor on the last row. */
    if (vga_row > 0) vga_row--;
}

/* ── Public API ──────────────────────────────────────────────────────────── */

void vga_init(void)
{
    vga_buf  = VGA_BASE;
    vga_col  = 0;
    vga_row  = 0;
    vga_attr = make_attr(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK); /* classic terminal */

    vga_clear();
}

void vga_set_color(vga_color_t fg, vga_color_t bg)
{
    vga_attr = make_attr(fg, bg);
}

void vga_set_cursor(uint8_t col, uint8_t row)
{
    if (col < VGA_WIDTH)  vga_col = col;
    if (row < VGA_HEIGHT) vga_row = row;
    update_hw_cursor();
}

void vga_clear(void)
{
    uint16_t blank = make_entry(' ', vga_attr);
    for (uint16_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_buf[i] = blank;
    }
    vga_col = 0;
    vga_row = 0;
    update_hw_cursor();
}

void vga_putchar(char c)
{
    switch (c) {

    case '\n':                      /* newline — go to next row, col 0 */
        vga_col = 0;
        vga_row++;
        break;

    case '\r':                      /* carriage return — col 0, same row */
        vga_col = 0;
        break;

    case '\t':                      /* tab — align to next 8-column stop */
        vga_col = (uint8_t)((vga_col + 8) & ~7u);
        if (vga_col >= VGA_WIDTH) {
            vga_col = 0;
            vga_row++;
        }
        break;

    case '\b':                      /* backspace — erase previous cell */
        if (vga_col > 0) {
            vga_col--;
        } else if (vga_row > 0) {
            vga_row--;
            vga_col = VGA_WIDTH - 1;
        }
        vga_buf[vga_row * VGA_WIDTH + vga_col] = make_entry(' ', vga_attr);
        break;

    default:                        /* printable character */
        vga_buf[vga_row * VGA_WIDTH + vga_col] = make_entry(c, vga_attr);
        vga_col++;
        if (vga_col >= VGA_WIDTH) {
            vga_col = 0;
            vga_row++;
        }
        break;
    }

    /* If we've gone past the last row, scroll. */
    if (vga_row >= VGA_HEIGHT) {
        scroll_up();
    }

    update_hw_cursor();
}

void printc(const char *str)
{
    if (!str) return;
    while (*str) {
        vga_putchar(*str++);
    }
}

void printc_at(const char *str, uint8_t col, uint8_t row)
{
    if (!str) return;

    /* Save current cursor. */
    uint8_t saved_col = vga_col;
    uint8_t saved_row = vga_row;

    vga_col = col < VGA_WIDTH  ? col : 0;
    vga_row = row < VGA_HEIGHT ? row : 0;

    while (*str) {
        vga_putchar(*str++);
    }

    /* Restore cursor. */
    vga_col = saved_col;
    vga_row = saved_row;
    update_hw_cursor();
}

void printc_hex(uint64_t value)
{
    static const char hex_digits[] = "0123456789ABCDEF";
    char buf[19];   /* "0x" + 16 hex digits + '\0' */
    int  idx = 18;

    buf[idx--] = '\0';

    /* Write 16 hex digits right-to-left. */
    for (int i = 0; i < 16; i++) {
        buf[idx--] = hex_digits[value & 0xF];
        value >>= 4;
    }
    buf[idx--] = 'x';
    buf[idx]   = '0';

    printc(&buf[idx]);
}


