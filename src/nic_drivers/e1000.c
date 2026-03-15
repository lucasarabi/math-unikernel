#include "e1000.h"
#include "../headers/io.h"
#include "../headers/display.h"
#include "../headers/hhdm_offset.h"
#include "../headers/pmm.h"
#include "../headers/vmm.h"

#define MMIO_MAPPED         "NIC: Intel I219-LM MMIO mapped.\n"
#define MAC_ADDRESS         "NIC: MAC Address: "
#define HANDSHAKE_COMPLETE  "NIC: Handshake complete.\n"

#define KB 1024

#define NUM_RX_DESCRIPTORS 32

#define RAL0      0x5400  // Receive Address Low (MAC bytes 0-3)
#define RAH0      0x5404  // Receive Address High (MAC bytes 4-5)
#define RDBAL     0x2800  // RX Descriptor Base Address Low
#define RDBAH     0x2804  // RX Descriptor Base Address High
#define RDLEN     0x2808  // RX Descriptor Length
#define RDH       0x2810  // RX Descriptor Head
#define RDT       0x2818  // RX Descriptor Tail
#define RCTL      0x0100  // Receive Control

static void print_mac_byte(uint8_t byte) {
    const char hex_chars[] = "0123456789ABCDEF";
    char str[3];
    str[0] = hex_chars[(byte >> 4) & 0x0F]; 
    str[1] = hex_chars[byte & 0x0F];        
    str[2] = '\0';
    PRINTS(str);
}

static volatile uint64_t mmio_base_virt; 
static volatile struct e1000_rx_descriptor* rx_ring_virt;
static uint8_t* rx_buffers_virt[NUM_RX_DESCRIPTORS];

void e1000_init(uint32_t bar0, uint8_t irq) {
    // Map MMIO
    uint64_t mmio_base_phys = bar0 & ~0xf;
    mmio_base_virt = mmio_base_phys + hhdm_offset;
    vmm_map_range(mmio_base_virt, mmio_base_phys, 128*KB, VMM_PRESENT | VMM_WRITEABLE | VMM_CACHE_DISABLE);
    
    // Get MAC address
    volatile uint32_t mac_lo = *(volatile uint32_t*) ( (uint8_t*)mmio_base_virt + RAL0 );
    volatile uint32_t mac_hi = *(volatile uint32_t*) ( (uint8_t*)mmio_base_virt + RAH0 );

    uint8_t mac_bytes[6];
    mac_bytes[0] = mac_lo & 0xff;
    mac_bytes[1] = (mac_lo >> 8) & 0xff;
    mac_bytes[2] = (mac_lo >> 16) & 0xff;
    mac_bytes[3] = (mac_lo >> 24) & 0xff;
    mac_bytes[4] = mac_hi & 0xff;
    mac_bytes[5] = (mac_hi >> 8) & 0xff;

    PRINTS(MAC_ADDRESS);
    for(uint8_t i = 0; i < 6; i++) {
        print_mac_byte(mac_bytes[i]);
        if(i < 5) PRINTS(":");
    }
    PRINTLN;

    // Prepare RX Descriptor Table    
    uint64_t rx_ring_phys = pmm_alloc();
    rx_ring_virt = (struct e1000_rx_descriptor*)(rx_ring_phys + hhdm_offset);

    for(uint8_t i = 0; i < NUM_RX_DESCRIPTORS; i++) {
        uint64_t buf_phys = pmm_alloc();

        rx_buffers_virt[i] = (uint8_t*)(buf_phys + hhdm_offset);

        rx_ring_virt[i].addr = buf_phys;
        rx_ring_virt[i].status = 0;
    }

    // Hardware handshake

    // Tell the NIC where the ring is (Physical Address)
    // Split the 64-bit physical address into two 32-bit MMIO register writes.
    *(volatile uint32_t*)((uint8_t*)mmio_base_virt + RDBAL) = (uint32_t)(rx_ring_phys & 0xFFFFFFFF);   
    *(volatile uint32_t*)((uint8_t*)mmio_base_virt + RDBAH) = (uint32_t)(rx_ring_phys >> 32);

    // Tell the NIC how big the ring is (in bytes)
    // 32 descriptors * 16 bytes per descriptor = 512 bytes
    *(volatile uint32_t*)((uint8_t*)mmio_base_virt + RDLEN) = NUM_RX_DESCRIPTORS * sizeof(struct e1000_rx_descriptor);

    // Set the Head and Tail pointers
    // Hardware drops packets at the Head. We tell it the Tail is at the end of the ring.
    *(volatile uint32_t*)((uint8_t*)mmio_base_virt + RDH) = 0;
    *(volatile uint32_t*)((uint8_t*)mmio_base_virt + RDT) = NUM_RX_DESCRIPTORS - 1;

    // Enable the Receiver
    // Bit 1 (EN) turns it on. Bit 26 (SECRC) strips the ethernet checksum so we only get the payload.
    *(volatile uint32_t*)((uint8_t*)mmio_base_virt + RCTL) = (1 << 1) | (1 << 26);

    PRINTS(HANDSHAKE_COMPLETE);
}
