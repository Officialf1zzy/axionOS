#include "../drivers/vga.h"
#include "../drivers/keyboard.h"
#include "game.h"

void main(void) 
{
vga_init();
keyboard_init();

printc("starting game");

}
