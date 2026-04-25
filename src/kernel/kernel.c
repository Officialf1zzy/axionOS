#include "../drivers/vga.h"

int strcmp(const char *a, const char *b)
{
    while (*a && *b) {
        if (*a != *b)
            return *a - *b;
        a++;
        b++;
    }
    return *a - *b;
}

void kernel_main(void)
{
    vga_init();

    printc("Kernel booted!\n");
    printc("VGA driver is working.\n\n");
    __asm__ volatile ("cli");

    printc("starting print test \n");

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
    printc("Example error message\n");

    printc("print test complete\n");
    //color output
    vga_set_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK);
    printc("[  OK  ] ");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    printc("started kernel\n");

    vga_set_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK);
    printc("[  OK  ] ");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    printc("started os\n");

    printc(" \n");

    vga_set_color(VGA_COLOR_BLUE, VGA_COLOR_BLACK);
    printc("Welcome to axionOS\n");

    vga_set_color(VGA_COLOR_CYAN, VGA_COLOR_BLACK);
    printc("Kernel base address : ");
    printc_hex(0xFFFFFFFF80000000ULL);
    printc("\n");
    //status bar
    vga_set_color(VGA_COLOR_BLACK, VGA_COLOR_LIGHT_GREY);
    printc_at("  axionOS v0.1  |  demo version                               "
              "              ", 0, VGA_HEIGHT - 1);

    //reset colors
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);

    //halt cpu
    for (;;) {
        __asm__ volatile ("hlt");
    }
}

void _start() 
{
    kernel_main();
}
