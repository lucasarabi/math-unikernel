#ifndef RTL8139_H
#define RTL8139_H

#include <stdint.h>

void rtl8139_init(uint32_t bar0);

// rtl8139 NIC API

uint8_t rtl8139_poll();

#endif