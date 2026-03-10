#include "e1000.h"
#include "../headers/io.h"
#include "../headers/display.h"
#include "../headers/hhdm_offset.h"
#include "../headers/pmm.h"

#define MMIO_MAPPED "NIC: Intel I219-LM MMIO mapped.\n"
#define MAC_ADDRESS "NIC: MAC Address: "

#define E1000_RAL0      0x5400  // Receive Address Low (MAC bytes 0-3)
#define E1000_RAH0      0x5404  // Receive Address High (MAC bytes 4-5)
#define E1000_RDBAL     0x2800  // RX Descriptor Base Address Low
#define E1000_RDLEN     0x2808  // RX Descriptor Length
#define E1000_RDH       0x2810  // RX Descriptor Head
#define E1000_RDT       0x2818  // RX Descriptor Tail
#define E1000_RCTL      0x0100  // Receive Control

uint64_t mmio_base_virt;

static void print_mac_byte(uint8_t byte) {
    const char hex_chars[] = "0123456789ABCDEF";
    char str[3];
    str[0] = hex_chars[(byte >> 4) & 0x0F]; 
    str[1] = hex_chars[byte & 0x0F];        
    str[2] = '\0';
    PRINTS(str);
}

void e1000_init(uint32_t bar0) {
    uint64_t mmio_base_phys = bar0 & ~0xf;
    mmio_base_virt = mmio_base_phys + hhdm_offset;

    PRINTS(MMIO_MAPPED);

    uint32_t mac_low = mmio_read32(E1000_RAL0);
    uint32_t mac_high = mmio_read32(E1000_RAH0);
    
    uint8_t mac[6];
    mac[0] = mac_low & 0xff;
    mac[1] = (mac_low >> 8) & 0xff;
    mac[2] = (mac_low >> 16) & 0xff;
    mac[3] = (mac_low >> 24) & 0xff;
    mac[4] = mac_high & 0xff;
    mac[5] = (mac_high >> 8) & 0xff;

    PRINTS(MAC_ADDRESS);
    for(int i=0; i<6; i++) {
        print_mac_byte(mac[i]);
        if(i < 5) PRINTS(":");
    }
    PRINTLN;



}