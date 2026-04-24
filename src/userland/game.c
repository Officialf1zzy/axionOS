#include "../drivers/vga.h"
#include "../drivers/keyboard.h"

void main(void) 
{
vga_init();
keyboard_init();

printc("starting game");

}
