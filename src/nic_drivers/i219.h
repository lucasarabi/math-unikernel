#ifndef I219_MU_H
#define I219_MU_H

#include <stdint.h>

// The formal Descriptor struct expected by the Intel I219-LM silicon.
// Identical layout to e1000 — the ring format did not change.
struct i219_rx_descriptor {
    uint64_t addr;      // Physical address of the receive buffer (set by you)
    uint16_t length;    // Size of received packet in bytes (set by the NIC)
    uint16_t csum;      // Hardware checksum (ignored)
    uint8_t  status;    // Bit 0 (DD) set by NIC when packet is written
    uint8_t  errors;    // Error flags (non-zero means drop the packet)
    uint16_t special;   // VLAN tag (ignored)
} __attribute__((packed));

void i219_init(uint32_t bar0_raw, uint8_t irq);
void i219_poll_rx();

#endif