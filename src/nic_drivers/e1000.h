#ifndef E1000_MU_H
#define E1000_MU_H

#include <stdint.h>

// The formal Descriptor struct expected by the Intel silicon
struct e1000_rx_descriptor {
    uint64_t addr;      // Physical address of the 2KB/4KB buffer (Set by YOU)
    uint16_t length;    // Size of the packet in bytes (Set by the NIC)
    uint16_t csum;      // Hardware checksum (We ignore this)
    uint8_t  status;    // Status flags (NIC sets Bit 0 to 1 when a packet arrives)
    uint8_t  errors;    // Error flags (NIC sets this if the packet is corrupted)
    uint16_t special;   // VLAN tags (We ignore this)
} __attribute__((packed));

void e1000_init(uint32_t bar0);

#endif
