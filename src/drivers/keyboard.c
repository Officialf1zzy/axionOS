/**
 * keyboard.c — Bare-metal PS/2 keyboard driver (Scancode Set 1)
 *
 * Interrupt flow
 * ──────────────
 *   1. User presses / releases a key.
 *   2. The keyboard controller asserts IRQ1.
 *   3. The PIC signals INT 33 to the CPU.
 *   4. CPU saves state and jumps to keyboard_isr_stub (ASM).
 *   5. Stub saves registers and calls keyboard_irq_handler() (C).
 *   6. Handler reads scancode, translates to ASCII, enqueues.
 *   7. Stub restores registers and executes IRET.
 *   8. Kernel consumes characters via keyboard_getchar().
 */

#include "keyboard.h"
#include "idt.h"
#include "pic.h"
#include "io.h"
#include <stddef.h>

/* ══════════════════════════════════════════════════════════════════════
 * Scancode Set 1 → ASCII lookup tables
 *
 * Index = make-code scancode (0x01-0x58).
 * 0x00 means "no printable character" (modifier key, function key, etc.).
 *
 * Two tables: unshifted and shifted.
 * ══════════════════════════════════════════════════════════════════════ */

/* Unshifted characters for scancodes 0x00-0x58 */
static const char scancode_table_lower[89] = {
/*00*/  0,
/*01*/  0x1B,  /* ESC        */
/*02*/  '1',
/*03*/  '2',
/*04*/  '3',
/*05*/  '4',
/*06*/  '5',
/*07*/  '6',
/*08*/  '7',
/*09*/  '8',
/*0A*/  '9',
/*0B*/  '0',
/*0C*/  '-',
/*0D*/  '=',
/*0E*/  '\b',  /* BACKSPACE  */
/*0F*/  '\t',  /* TAB        */
/*10*/  'q',
/*11*/  'w',
/*12*/  'e',
/*13*/  'r',
/*14*/  't',
/*15*/  'y',
/*16*/  'u',
/*17*/  'i',
/*18*/  'o',
/*19*/  'p',
/*1A*/  '[',
/*1B*/  ']',
/*1C*/  '\n',  /* ENTER      */
/*1D*/  0,     /* L-CTRL     */
/*1E*/  'a',
/*1F*/  's',
/*20*/  'd',
/*21*/  'f',
/*22*/  'g',
/*23*/  'h',
/*24*/  'j',
/*25*/  'k',
/*26*/  'l',
/*27*/  ';',
/*28*/  '\'',
/*29*/  '`',
/*2A*/  0,     /* L-SHIFT    */
/*2B*/  '\\',
/*2C*/  'z',
/*2D*/  'x',
/*2E*/  'c',
/*2F*/  'v',
/*30*/  'b',
/*31*/  'n',
/*32*/  'm',
/*33*/  ',',
/*34*/  '.',
/*35*/  '/',
/*36*/  0,     /* R-SHIFT    */
/*37*/  '*',   /* numpad *   */
/*38*/  0,     /* L-ALT      */
/*39*/  ' ',
/*3A*/  0,     /* CAPS LOCK  */
/*3B*/  0,     /* F1         */
/*3C*/  0,     /* F2         */
/*3D*/  0,     /* F3         */
/*3E*/  0,     /* F4         */
/*3F*/  0,     /* F5         */
/*40*/  0,     /* F6         */
/*41*/  0,     /* F7         */
/*42*/  0,     /* F8         */
/*43*/  0,     /* F9         */
/*44*/  0,     /* F10        */
/*45*/  0,     /* NUM LOCK   */
/*46*/  0,     /* SCROLL LCK */
/*47*/  '7',   /* numpad 7   */
/*48*/  '8',   /* numpad 8   */
/*49*/  '9',   /* numpad 9   */
/*4A*/  '-',   /* numpad -   */
/*4B*/  '4',   /* numpad 4   */
/*4C*/  '5',   /* numpad 5   */
/*4D*/  '6',   /* numpad 6   */
/*4E*/  '+',   /* numpad +   */
/*4F*/  '1',   /* numpad 1   */
/*50*/  '2',   /* numpad 2   */
/*51*/  '3',   /* numpad 3   */
/*52*/  '0',   /* numpad 0   */
/*53*/  '.',   /* numpad .   */
/*54*/  0,
/*55*/  0,
/*56*/  0,
/*57*/  0,     /* F11        */
/*58*/  0,     /* F12        */
};

/* Shifted characters — same layout, different values */
static const char scancode_table_upper[89] = {
/*00*/  0,
/*01*/  0x1B,
/*02*/  '!',
/*03*/  '@',
/*04*/  '#',
/*05*/  '$',
/*06*/  '%',
/*07*/  '^',
/*08*/  '&',
/*09*/  '*',
/*0A*/  '(',
/*0B*/  ')',
/*0C*/  '_',
/*0D*/  '+',
/*0E*/  '\b',
/*0F*/  '\t',
/*10*/  'Q',
/*11*/  'W',
/*12*/  'E',
/*13*/  'R',
/*14*/  'T',
/*15*/  'Y',
/*16*/  'U',
/*17*/  'I',
/*18*/  'O',
/*19*/  'P',
/*1A*/  '{',
/*1B*/  '}',
/*1C*/  '\n',
/*1D*/  0,
/*1E*/  'A',
/*1F*/  'S',
/*20*/  'D',
/*21*/  'F',
/*22*/  'G',
/*23*/  'H',
/*24*/  'J',
/*25*/  'K',
/*26*/  'L',
/*27*/  ':',
/*28*/  '"',
/*29*/  '~',
/*2A*/  0,
/*2B*/  '|',
/*2C*/  'Z',
/*2D*/  'X',
/*2E*/  'C',
/*2F*/  'V',
/*30*/  'B',
/*31*/  'N',
/*32*/  'M',
/*33*/  '<',
/*34*/  '>',
/*35*/  '?',
/*36*/  0,
/*37*/  '*',
/*38*/  0,
/*39*/  ' ',
/*3A*/  0,
/*3B*/  0, /*3C*/  0, /*3D*/  0, /*3E*/  0,
/*3F*/  0, /*40*/  0, /*41*/  0, /*42*/  0,
/*43*/  0, /*44*/  0,
/*45*/  0, /*46*/  0,
/*47*/  '7', /*48*/  '8', /*49*/  '9', /*4A*/  '-',
/*4B*/  '4', /*4C*/  '5', /*4D*/  '6', /*4E*/  '+',
/*4F*/  '1', /*50*/  '2', /*51*/  '3', /*52*/  '0',
/*53*/  '.',
/*54*/  0, /*55*/  0, /*56*/  0,
/*57*/  0, /*58*/  0,
};

/* ══════════════════════════════════════════════════════════════════════
 * Driver state  (file-scope, zero-initialized at boot)
 * ══════════════════════════════════════════════════════════════════════ */

static keyboard_state_t kb;

/* ══════════════════════════════════════════════════════════════════════
 * Ring-buffer helpers  (lock-free SPSC: one producer in IRQ, one consumer)
 * ══════════════════════════════════════════════════════════════════════ */

/* Next index in the circular buffer */
#define BUF_NEXT(i)  (((i) + 1) & (KB_BUFFER_SIZE - 1))

static inline bool buf_full(void) {
    return BUF_NEXT(kb.tail) == kb.head;
}

static inline bool buf_empty(void) {
    return kb.head == kb.tail;
}

/* Enqueue one character — called from IRQ context */
static inline void buf_push(char c) {
    if (!buf_full()) {
        kb.buf[kb.tail] = c;
        kb.tail = BUF_NEXT(kb.tail);
    }
    /* If full, silently drop — the kernel is not consuming fast enough */
}

/* Dequeue one character — called from kernel/user context */
static inline char buf_pop(void) {
    char c = kb.buf[kb.head];
    kb.head = BUF_NEXT(kb.head);
    return c;
}

/* ══════════════════════════════════════════════════════════════════════
 * Scancode → ASCII translation
 * ══════════════════════════════════════════════════════════════════════ */

/**
 * translate_scancode() — Convert a Set-1 make/break code into an ASCII
 * character (or 0 for non-printable / release events).
 *
 * @sc : raw byte from port 0x60
 * Returns the ASCII character to enqueue, or 0.
 */
static char translate_scancode(uint8_t sc) {
    /* ── Extended-key prefix ─────────────────────────────────────── */
    if (sc == 0xE0) {
        kb.modifiers |= 0;      /* nothing to record yet */
        kb.extended = true;     /* next byte is the extended code */
        return 0;
    }

    bool is_break = (sc & 0x80) != 0;   /* bit 7 → key released */
    uint8_t code  = sc & 0x7F;           /* strip break bit       */

    /* ── Extended scancodes (0xE0 prefix) ───────────────────────── */
    if (kb.extended) {
        kb.extended = false;
        switch (code) {
            case 0x1D:  /* R-CTRL */
                if (is_break) kb.modifiers &= ~KB_MOD_CTRL;
                else          kb.modifiers |=  KB_MOD_CTRL;
                return 0;
            case 0x38:  /* R-ALT  */
                if (is_break) kb.modifiers &= ~KB_MOD_ALT;
                else          kb.modifiers |=  KB_MOD_ALT;
                return 0;
            case 0x48:  /* UP arrow    */
                return is_break ? 0 : 0;  /* extend here if needed */
            case 0x50:  /* DOWN arrow  */
                return is_break ? 0 : 0;
            case 0x4B:  /* LEFT arrow  */
                return is_break ? 0 : 0;
            case 0x4D:  /* RIGHT arrow */
                return is_break ? 0 : 0;
            case 0x1C:  /* numpad ENTER — treat same as main ENTER */
                return is_break ? 0 : '\n';
            default:
                return 0;
        }
    }

    /* ── Standard (non-extended) scancodes ──────────────────────── */

    /* Handle modifier key presses / releases first */
    switch (code) {
        case 0x2A:  /* L-SHIFT */
            if (is_break) kb.modifiers &= ~KB_MOD_LSHIFT;
            else          kb.modifiers |=  KB_MOD_LSHIFT;
            return 0;
        case 0x36:  /* R-SHIFT */
            if (is_break) kb.modifiers &= ~KB_MOD_RSHIFT;
            else          kb.modifiers |=  KB_MOD_RSHIFT;
            return 0;
        case 0x1D:  /* L-CTRL */
            if (is_break) kb.modifiers &= ~KB_MOD_CTRL;
            else          kb.modifiers |=  KB_MOD_CTRL;
            return 0;
        case 0x38:  /* L-ALT */
            if (is_break) kb.modifiers &= ~KB_MOD_ALT;
            else          kb.modifiers |=  KB_MOD_ALT;
            return 0;
        case 0x3A:  /* CAPS LOCK — toggle on press only */
            if (!is_break) kb.modifiers ^= KB_MOD_CAPSLOCK;
            return 0;
        case 0x45:  /* NUM LOCK */
            if (!is_break) kb.modifiers ^= KB_MOD_NUMLOCK;
            return 0;
        case 0x46:  /* SCROLL LOCK */
            if (!is_break) kb.modifiers ^= KB_MOD_SCRLOCK;
            return 0;
    }

    /* Ignore break codes for all other keys */
    if (is_break)
        return 0;

    /* Bounds-check against our table */
    if (code >= sizeof(scancode_table_lower))
        return 0;

    /* Determine shift state (Shift XOR CapsLock for letters) */
    bool shift = (kb.modifiers & (KB_MOD_LSHIFT | KB_MOD_RSHIFT)) != 0;
    bool caps  = (kb.modifiers & KB_MOD_CAPSLOCK) != 0;

    char lower = scancode_table_lower[code];
    char upper = scancode_table_upper[code];

    if (lower == 0)
        return 0;   /* non-printable key — F-key, etc. */

    /* CapsLock only flips alphabetic characters, not symbols */
    bool is_alpha = (lower >= 'a' && lower <= 'z');
    bool use_upper = shift;
    if (is_alpha) use_upper = shift ^ caps;

    return use_upper ? upper : lower;
}

/* ══════════════════════════════════════════════════════════════════════
 * PS/2 controller helpers
 * ══════════════════════════════════════════════════════════════════════ */

/* Wait until the controller's input buffer is empty (safe to write) */
static void ps2_wait_write(void) {
    int timeout = 100000;
    while ((inb(PS2_STATUS_PORT) & PS2_STATUS_IBF) && --timeout)
        io_wait();
}

/* Wait until the output buffer has data (safe to read) */
static void ps2_wait_read(void) {
    int timeout = 100000;
    while (!(inb(PS2_STATUS_PORT) & PS2_STATUS_OBF) && --timeout)
        io_wait();
}

/* Send a command byte to the PS/2 controller */
static void ps2_send_cmd(uint8_t cmd) {
    ps2_wait_write();
    outb(PS2_CMD_PORT, cmd);
}

/* ══════════════════════════════════════════════════════════════════════
 * Public API
 * ══════════════════════════════════════════════════════════════════════ */

void keyboard_init(void) {
    /* 1. Disable keyboard port while we configure the controller */
    ps2_send_cmd(PS2_CMD_DISABLE_PORT1);

    /* 2. Flush any stale bytes sitting in the output buffer */
    while (inb(PS2_STATUS_PORT) & PS2_STATUS_OBF)
        inb(PS2_DATA_PORT);

    /* 3. Read the current controller config byte */
    ps2_send_cmd(PS2_CMD_READ_CONFIG);
    ps2_wait_read();
    uint8_t config = inb(PS2_DATA_PORT);

    /* 4. Enable IRQ1 (bit 0) and make sure keyboard translation is on (bit 6) */
    config |= (1 << 0);   /* enable IRQ1                */
    config |= (1 << 6);   /* enable scancode translation */
    config &= ~(1 << 4);  /* clear "disable keyboard"  */

    /* 5. Write the updated config back */
    ps2_send_cmd(PS2_CMD_WRITE_CONFIG);
    ps2_wait_write();
    outb(PS2_DATA_PORT, config);

    /* 6. Re-enable the keyboard port */
    ps2_send_cmd(PS2_CMD_ENABLE_PORT1);

    /* 7. Initialise driver state */
    kb.modifiers = 0;
    kb.extended  = false;
    kb.head      = 0;
    kb.tail      = 0;

    /* 8. Install ISR in the IDT (INT 33 = 0x21) */
    /* idt_set_gate(INT_KEYBOARD,
    /*             (uint32_t)keyboard_isr_stub,
    /*             0x08,                  /* kernel code segment */
    /*             IDT_TYPE_INTERRUPT_32);

    /* 9. Unmask IRQ1 in the PIC so interrupts start arriving */
    pic_clear_mask(IRQ_KEYBOARD);
}

/**
 * keyboard_irq_handler — Invoked by keyboard_isr_stub on every IRQ1.
 *
 * Reading port 0x60 acknowledges the interrupt at the keyboard controller
 * level; sending EOI to the PIC acknowledges it at the PIC level.
 */
void keyboard_irq_handler(void) {
    /* Always read the scancode — even if we don't use it.
     * Failing to read clears the OBF flag and acknowledges the interrupt
     * at the controller; without this the next IRQ1 will never fire. */
    uint8_t scancode = inb(PS2_DATA_PORT);

    char c = translate_scancode(scancode);
    if (c != 0)
        buf_push(c);

    /* Acknowledge the interrupt at the PIC */
    pic_send_eoi(IRQ_KEYBOARD);
}

char keyboard_getchar(void) {
    if (buf_empty()) return 0;
    return buf_pop();
}

bool keyboard_available(void) {
    return !buf_empty();
}

char keyboard_getchar_blocking(void) {
    /* Spin with HLT to avoid burning CPU — HLT wakes on the next interrupt */
    while (!keyboard_available())
        __asm__ volatile ("hlt");
    return buf_pop();
}

uint8_t keyboard_get_modifiers(void) {
    return kb.modifiers;
}
