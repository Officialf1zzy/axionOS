#include "..\drivers\vga.h"
#include "..\drivers\keyboard.h"

void _start() 
{
    kernel_main();
}

void kernel_main(void)
{
    vga_init();
    keyboard_init();

    printc("Kernel booted!\n");
    printc("VGA driver is working.\n\n");

    //color output
    vga_set_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK);
    printc("[  OK  ] ");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    printc("Memory subsystem initialised\n");

    vga_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    printc("[ WARN ] ");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    printc("No ACPI tables found\n");

    vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
    printc("[  ERR ] ");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    printc("Example error message\n\n");

    //numbers
    vga_set_color(VGA_COLOR_CYAN, VGA_COLOR_BLACK);
    printc("Kernel base address : ");
    printc_hex(0xFFFFFFFF80000000ULL);
    printc("\n");

    printc("Page size           : ");
    printc_dec(4096);
    printc(" bytes\n\n");

    //status bar
    vga_set_color(VGA_COLOR_BLACK, VGA_COLOR_LIGHT_GREY);
    printc_at("  MyOS v0.1  |  Press any key...                               "
              "              ", 0, VGA_HEIGHT - 1);

    //reset colors
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    printc("Kernel initialisation complete.\n");

    //halt
    for (;;) {
        __asm__ volatile ("hlt");
    }
}
