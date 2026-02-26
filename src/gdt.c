#include "headers/gdt.h"

#define NULL_ACCESS_BYTE    0x00
#define NULL_FLAGS          0x0
#define CODE_ACCESS_BYTE    0x9a
#define CODE_FLAGS          0xa
#define DATA_ACCESS_BYTE    0x92
#define DATA_FLAGS          0x0

struct gdt_context gdt;

descriptor create_descriptor(uint8_t access_byte, uint8_t flags) {
    return ((uint64_t)access_byte << 40) | ((uint64_t)flags <<52);
}

void gdt_init() {
    gdt.entries[0] = create_descriptor(NULL_ACCESS_BYTE, NULL_FLAGS);
    gdt.entries[1] = create_descriptor(CODE_ACCESS_BYTE, CODE_FLAGS);
    gdt.entries[2] = create_descriptor(DATA_ACCESS_BYTE, DATA_FLAGS);

    gdt.pointer.base = (uint64_t)gdt.entries;
    gdt.pointer.limit = (uint16_t)(sizeof(gdt.entries) - 1);

    load_gdt(&gdt.pointer);
}