#include "../drivers/vga.h"
#include "../drivers/pic.h"
#include "../drivers/idt.h"
#include "../drivers/io.h"
#include "../drivers/gdt.h"
#include "../drivers/keyboard.h"
#include "../drivers/timer.h"

#define INPUT_BUF 128

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
	timer_init();
    gdt_init();
    idt_init();
    pic_remap();
    keyboard_init();
	pic_clear_mask(0);
    pic_clear_mask(1);
	__asm__ volatile ("sti");

    printc("\n \n \n \n \n \n \n \n \n \n \n");
	printc("               booting axoinOS");
	timer_sleep(8000);

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

    //reset colors
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    printc("Kernel initialisation complete.\n");

	timer_sleep(3000);
	vga_clear();
    
    __asm__ volatile ("sti");

    char usrinput[INPUT_BUF];
    int idx = 0;

    printc("axoin> ");

    while (1) 
    {
	    if (keyboard_available()) {
	        char c = keyboard_getchar();
            char str[] = { c, '\0' };
            printc(str);

            if (c == '\n') {
                usrinput[idx] = '\0';
                if (strcmp(usrinput, "help") == 0) {
                    printc("commands:\n");
                    printc("help\n");
                }

                if (strcmp(usrinput, "sysinfo") == 0) {
                    printc("version: axoinOS 1.0 beta\n");
                    printc("bootloader: grub multiboot2\n");
                }

                if (strcmp(usrinput, "userland") == 0) {
                    printc("no userland\n");
                }

                idx = 0;
                printc("axion> ");
            }
            else {
                if (idx < INPUT_BUF - 1) {
                    usrinput[idx++] = c;
                }
            }
	    }
	    else {
	         __asm__ volatile ("hlt");
	    }
    }
   
    //halt
    for (;;) {
        __asm__ volatile ("hlt");
    }
}

void _start() 
{
    kernel_main();
}
