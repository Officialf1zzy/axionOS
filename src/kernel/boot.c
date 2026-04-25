#include <stdint.h>

__attribute__((section(".multiboot"), used))
static const uint32_t multiboot_header[] = {
    0xE85250D6,                 // magic
    0,                          // architecture (i386 = 0)
    16,                         // header length
    -(0xE85250D6 + 0 + 16),     // checksum

    // END TAG (required)
    0, 0, 8, 0
};