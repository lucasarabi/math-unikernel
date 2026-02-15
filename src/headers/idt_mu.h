#ifndef IDT_MU_H
#define IDT_MU_H

#include <stdint.h>

struct idt_entry {
    uint16_t isr_low;           // Lower 16 bits of the ISR's address
    uint16_t kernel_cs;         // The GDT segment selector 
    uint8_t ist;                // Interrupt stack table offset
    uint8_t attributes;         // Type and attributes (flags)
    uint16_t isr_mid;           // The middle 16 bits of the ISR's address
    uint32_t isr_high;          // The higher 32 bits of the ISR's address
    uint32_t reserved;          // Set to zero
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

struct idt_context {
    struct idt_entry entries[256];
    struct idt_ptr pointer;
    uint64_t total_interrupts;
};

extern struct idt_context idt;
extern void* isr_stub_table[];

void idt_init();

#endif