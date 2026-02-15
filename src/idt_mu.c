#include "headers/idt_mu.h"
#include "headers/lib_mu.h"
#include "headers/io_mu.h"

#define GDT_KERNEL 0x08

#define MASK_16 0xffff
#define MASK_32 0xffffffff

struct idt_context idt; 

void idt_set_descriptor(uint8_t vector, uint64_t virt_addr, uint8_t flags) {
    struct idt_entry* entry = &idt.entries[vector];

    entry->isr_low = (uint16_t)(virt_addr & MASK_16);
    entry->isr_mid = (uint16_t)((virt_addr >> 16) & MASK_16);
    entry->isr_high = (uint32_t)((virt_addr >> 32) & MASK_32);

    entry->kernel_cs = GDT_KERNEL;
    entry->attributes = flags;
    entry->reserved = 0;
    entry->ist = 0;
}

void idt_init() {


}