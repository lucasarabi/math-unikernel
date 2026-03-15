#ifndef IDT_MU_H
#define IDT_MU_H

#include <stdint.h>
#define IDT_INIT_SUCCESS    (1<<2)

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

struct interrupt_frame {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8; // Registers pushed by isr_common (reverse order of pushq)
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;

    // Pushed by the isr_stub macro
    uint64_t interrupt_number;
    uint64_t error_code;

    // Pushed automatically by the CPU hardware
    uint64_t rip, cs, rflags, rsp, ss;
} __attribute__((packed));

extern struct idt_context idt;
extern void* isr_stub_table[];

// Interrupt Descriptor Table API
void load_idt(struct idt_ptr* ptr);
uint8_t idt_init();
void interrupt_dispatcher(struct interrupt_frame* frame);
void register_irq_handler(uint8_t irq, void* handler);

#endif
