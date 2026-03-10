#ifndef E1000_MU_H
#define E1000_MU_H

#include <stdint.h>

// The formal Descriptor struct expected by the Intel silicon
struct e1000_rx_desc {
    uint64_t addr;      // Physical address of the 2KB buffer
    uint16_t length;    // Packet length populated by the NIC
    uint16_t csum;      
    uint8_t  status;    // Bit 0 (DD) is set by NIC when packet is ready!
    uint8_t  errors;    
    uint16_t special;
} __attribute__((packed));


void e1000_init(uint32_t bar0);

#endif
