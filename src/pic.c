#include "headers/pic.h"
#include "headers/io.h"

#define PIC1_COMMAND    0x20
#define PIC1_DATA       0x21

#define PIC2_COMMAND    0xa0
#define PIC2_DATA       0xa1

#define PIC_EOI         0x20
#define ICW1_INIT       0x11    // Init Command Word 1
#define ICW4_8086       0x01    // Init Command Word 4

void pic_remap(uint8_t offset1, uint8_t offset2) {
    uint8_t mask1, mask2;

    mask1 = inb(PIC1_DATA);
    mask2 = inb(PIC2_DATA);

    // ICW 1
    outb(PIC1_COMMAND, ICW1_INIT);
    outb(PIC2_COMMAND, ICW1_INIT);

    // ICW 2
    outb(PIC1_DATA, offset1);
    outb(PIC2_DATA, offset2);

    // ICW 3
    outb(PIC1_DATA, 4);
    outb(PIC2_DATA, 2);

    // ICW 4
    outb(PIC1_DATA, ICW4_8086);
    outb(PIC2_DATA, ICW4_8086);

    outb(PIC1_DATA, mask1);
    outb(PIC2_DATA, mask2);
}

// Unblock only desired IRQ
void pic_unmask(uint8_t irq) {
    uint16_t port;
    uint8_t value;

    if(irq < 8) {
        port = PIC1_DATA;
    }
    else {
        port = PIC2_DATA;
        irq -= 8;

        outb(PIC1_DATA, inb(PIC1_DATA) & ~(1<<2));
    }

    value = inb(port) & ~(1 << irq);
    outb(port, value);
}

// End interrupt
void pic_send_eoi(uint8_t irq) {
    if(irq >= 8) {
        outb(PIC2_COMMAND, PIC_EOI);
    }
    outb(PIC1_COMMAND, PIC_EOI);
}