#ifndef PCI_MU_H
#define PCI_MU_H

#include <stdint.h>
#define PCI_INIT_SUCCESS (1<<8)

uint32_t pci_read_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
uint16_t pci_scan_bus();

#endif