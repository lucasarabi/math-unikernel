#include "e1000.h"
#include "../headers/io.h"
#include "../headers/display.h"
#include "../headers/hhdm_offset.h"
#include "../headers/pmm.h"
#include "../headers/vmm.h"

#define MMIO_MAPPED "NIC: Intel I219-LM MMIO mapped.\n"
#define MAC_ADDRESS "NIC: MAC Address: "

#define NUM_RX_DESCRIPTORS 32

#define RAL0      0x5400  // Receive Address Low (MAC bytes 0-3)
#define RAH0      0x5404  // Receive Address High (MAC bytes 4-5)
#define RDBAL     0x2800  // RX Descriptor Base Address Low
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

void e1000_init(uint32_t bar0) {
    // Map MMIO
    uint64_t mmio_base_phys = bar0 & ~0xf;
    // Map 128KB (32 pages of 4KB) of MMIO space
    for (uint64_t i = 0; i < 32; i++) {
        uint64_t offset = i * 4096;
        vmm_map_virt_to_phys(mmio_base_virt + offset, mmio_base_phys + offset, VMM_PRESENT | VMM_WRITEABLE | VMM_CACHE_DISABLE);
    }
    PRINTS(MMIO_MAPPED);

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

    

}
