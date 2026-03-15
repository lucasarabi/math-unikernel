#ifndef PIC_MU_H
#define PIC_MU_H

#include <stdint.h>

// Programmable Interrupt Controller API
void pic_remap(uint8_t offset1, uint8_t offset2);
void pic_unmask(uint8_t irq);
void pic_send_eoi(uint8_t irq);

#endif