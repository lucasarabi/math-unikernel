#include "headers/idt.h"
#include "headers/lib.h"
#include "headers/display.h"

#define GDT_KERNEL 0x08 
#define FULL_KERNEL_AUTHORITY 0x8e

#define MASK_16 0xffff
#define MASK_32 0xffffffff

struct idt_context idt; 

void* irq_handlers[16] = {0};

void register_irq_handler(uint8_t irq, void* handler) {
    if (irq < 16) {
        irq_handlers[irq] = handler;
    }
}

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

uint8_t idt_init() {
    memset(&idt.entries, 0, sizeof(idt.entries));

    for(uint8_t vector = 0; vector < 48; vector++) {
        idt_set_descriptor(vector, (uint64_t)isr_stub_table[vector], FULL_KERNEL_AUTHORITY);
    }

    idt.total_interrupts = 0;
    idt.pointer.limit = (uint16_t)(sizeof(idt.entries) - 1);
    idt.pointer.base = (uint64_t)idt.entries;

    load_idt(&idt.pointer);

    return IDT_INIT_SUCCESS;
}

void interrupt_dispatcher(struct interrupt_frame* frame) {
    if (frame->interrupt_number < 32) {
        idt.total_interrupts++;

        PRINTS("\n--- !!! MATH UNIKERNEL PANIC !!! ---\n");
        PRINTS("Exception Vector: "); PRINTD(frame->interrupt_number); PRINTLN;
        PRINTS("Error Code:       "); PRINTH(frame->error_code); PRINTLN;
        PRINTS("RIP (Address):    "); PRINTH(frame->rip); PRINTLN;

        PRINTS("\n--- REGISTER SNAPSHOT ---\n");
        PRINTF("RAX:", frame->rax); PRINTF("RBX:", frame->rbx); PRINTLN;
        PRINTF("RCX:", frame->rcx); PRINTF("RDX:", frame->rdx); PRINTLN;
        PRINTF("RDI:", frame->rdi); PRINTF("RSI:", frame->rsi); PRINTLN;

        PRINTS("\nHalting system to prevent data corruption...\n");
        hcf();
    }
    else if (frame->interrupt_number >= 32 && frame->interrupt_number <= 47) {
        // --- HARDWARE IRQ (HANDLE AND RESUME) ---
        uint8_t irq = frame->interrupt_number - 32;

        // Run the driver's function if it registered one
        void (*handler)() = irq_handlers[irq];
        if (handler) {
            handler();
        }

        // Acknowledge the PIC so it doesn't block future interrupts
        pic_send_eoi(irq);
    }
}

