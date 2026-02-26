#ifndef GDT_MU_H
#define GDT_MU_H

#include <stdint.h>

typedef uint64_t descriptor;

struct gdt_ptr {
    uint16_t limit;
    uint64_t base;
}__attribute__((packed));

struct gdt_context {
    descriptor entries[3];
    struct gdt_ptr pointer;
};

extern struct gdt_context gdt;

void load_gdt(struct gdt_ptr* ptr);
void gdt_init();

#endif