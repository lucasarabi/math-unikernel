#ifndef E1000_MU_H
#define E1000_MU_H

#include <stdint.h>

#define NUM_RX_DESCRIPTORS 32

// The formal Descriptor struct expected by the Intel silicon
struct e1000_rx_desc {
    uint64_t addr;      // Physical address of the 2KB buffer
    uint16_t length;    // Packet length populated by the NIC
    uint16_t csum;      
    uint8_t  status;    // Bit 0 (DD) is set by NIC when packet is ready!
    uint8_t  errors;    
    uint16_t special;
} __attribute__((packed));

extern uint64_t mmio_base_virt;
static struct e1000_rx_desc* rx_ring_virt;
static uint8_t* rx_buffers_virt[NUM_RX_DESCRIPTORS];

static inline void mmio_write32(uint32_t offset, uint32_t val) {
    *((volatile uint32_t*)(mmio_base_virt + offset)) = val;
}
static inline uint32_t mmio_read32(uint32_t offset) {
    return *((volatile uint32_t*)(mmio_base_virt + offset));
}

void e1000_init(uint32_t bar0);

#endif