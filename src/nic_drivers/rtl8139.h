#ifndef RTL8139_H
#define RTL8139_H

#include <stdint.h>

// rtl8139 NIC API
void rtl8139_init(uint32_t bar0, uint8_t irq);
void rtl8139_poll();

#endif