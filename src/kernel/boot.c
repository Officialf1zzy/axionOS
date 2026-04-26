#include <stdint.h>

__attribute__((section(".multiboot"), used))
static const uint32_t multiboot_header[] = {
    0xE85250D6,                 
    0,                          
    16,                         
    -(0xE85250D6 + 0 + 16),     

    0, 0, 8, 0
};
