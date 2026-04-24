/* AxionOS Linker Script */
/* Target: x86 32-bit, GRUB Multiboot */

ENTRY(_start)

SECTIONS
{
    /* Kernel loads at 1MB — standard for Multiboot */
    . = 0x00100000;

    /* ── Text (code) ─────────────────────────────── */
    .text ALIGN(4K) :
    {
        *(.multiboot)   /* Multiboot header must come first */
        *(.text)
        *(.text.*)
    }

    /* ── Read-only data ──────────────────────────── */
    .rodata ALIGN(4K) :
    {
        *(.rodata)
        *(.rodata.*)
    }

    /* ── Initialised data ────────────────────────── */
    .data ALIGN(4K) :
    {
        *(.data)
        *(.data.*)
    }

    /* ── Uninitialised data (BSS) ────────────────── */
    .bss ALIGN(4K) :
    {
        _bss_start = .;
        *(COMMON)
        *(.bss)
        *(.bss.*)
        _bss_end = .;
    }

    /* Expose kernel end address so you can use it in C */
    _kernel_end = .;

    /* Discard everything the kernel doesn't need */
    /DISCARD/ :
    {
        *(.comment)
        *(.eh_frame)
        *(.note*)
    }
}
