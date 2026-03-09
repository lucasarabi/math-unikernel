#ifndef RTL8139_H
#define RTL8139_H

#include <stdint.h>

void rtl8139_init(uint32_t bar0);
uint8_t read_ethernet();

#endif