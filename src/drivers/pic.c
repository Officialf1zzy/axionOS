#include "pic.h"
#include "io.h"

/* ICW = Initialization Command Word */
#define ICW1_ICW4       0x01    /* ICW4 needed            */
#define ICW1_SINGLE     0x02    /* single (not cascade)   */
#define ICW1_INTERVAL4  0x04    /* call address interval 4 */
#define ICW1_LEVEL      0x08    /* level-triggered mode   */
#define ICW1_INIT       0x10    /* begin initialisation   */

#define ICW4_8086       0x01    /* 8086/88 mode (vs MCS-80) */
#define ICW4_AUTO       0x02    /* auto EOI                 */
#define ICW4_BUF_SLAVE  0x08    /* buffered slave           */
#define ICW4_BUF_MASTER 0x0C    /* buffered master          */
#define ICW4_SFNM       0x10    /* special fully nested     */

void pic_remap(void) {
    /* Save current masks */
    uint8_t mask1 = inb(PIC1_DATA);
    uint8_t mask2 = inb(PIC2_DATA);

    /* Start initialisation sequence (cascade mode) */
    outb(PIC1_CMD,  ICW1_INIT | ICW1_ICW4);  io_wait();
    outb(PIC2_CMD,  ICW1_INIT | ICW1_ICW4);  io_wait();

    /* ICW2: new vector offsets */
    outb(PIC1_DATA, PIC1_OFFSET);  io_wait();
    outb(PIC2_DATA, PIC2_OFFSET);  io_wait();

    /* ICW3: master has slave on IRQ2; slave ID = 2 */
    outb(PIC1_DATA, 0x04);  io_wait();   /* master: slave at IR2 */
    outb(PIC2_DATA, 0x02);  io_wait();   /* slave:  cascade identity */

    /* ICW4: 8086 mode */
    outb(PIC1_DATA, ICW4_8086);  io_wait();
    outb(PIC2_DATA, ICW4_8086);  io_wait();

    /* Mask ALL IRQs after remap.
     * Do NOT restore the BIOS/GRUB masks — they may leave IRQ2-7 (or slave
     * IRQ8-15) unmasked.  Those lines have no IDT handlers here, so an
     * unmasked stray IRQ would triple-fault the CPU and cause a reset loop.
     * Individual drivers must explicitly call pic_clear_mask() for the IRQs
     * they handle (e.g. keyboard_init() calls pic_clear_mask(IRQ_KEYBOARD)).
     */
    (void)mask1;
    (void)mask2;
    outb(PIC1_DATA, 0xFF);   /* mask all master IRQs (0-7)  */
    outb(PIC2_DATA, 0xFF);   /* mask all slave  IRQs (8-15) */
}

void pic_send_eoi(uint8_t irq) {
    if (irq >= 8)
        outb(PIC2_CMD, PIC_EOI);   /* slave must also be notified */
    outb(PIC1_CMD, PIC_EOI);
}

void pic_set_mask(uint8_t irq) {
    uint16_t port;
    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    outb(port, inb(port) | (1 << irq));
}

void pic_clear_mask(uint8_t irq) {
    uint16_t port;
    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    outb(port, inb(port) & ~(1 << irq));
}