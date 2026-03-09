#include "rtl8139.h"
#include "../headers/io.h"
#include "../headers/display.h"
#include "../headers/hhdm_offset.h"
#include "../headers/pmm.h"

#define DRIVER_LOADED           "NIC: RTL8139 driver loaded. BAR0 address: "
#define RESET_COMPLETE          "NIC: Reset complete.\n"
#define MAC_ADDRESS             "NIC: MAC Address: "
#define RX_BUFFER_LISTENING     "NIC: RX Buffer configured and listening.\n"

void print_mac_byte(uint8_t byte) {
    const char hex_chars[] = "0123456789ABCDEF";
    char str[3];
    str[0] = hex_chars[(byte >> 4) & 0x0F]; 
    str[1] = hex_chars[byte & 0x0F];        
    str[2] = '\0';
    PRINTS(str);
}

uint8_t* rx_buffer;
static uint32_t global_bar0;

void rtl8139_init(uint32_t bar0) {
    global_bar0 = bar0;
    PRINTS(DRIVER_LOADED);
    PRINTH(bar0);
    PRINTLN;

    // Reset NIC
    outb(bar0 + 0x37, 0x10);
    while((inb(bar0+0x37) & 0x10) != 0) {}
    PRINTS(RESET_COMPLETE);

    // Read MAC address
    PRINTS(MAC_ADDRESS);
    for(uint8_t i = 0; i < 6; i++) {
        uint8_t mac_byte = inb(bar0 + 0x00 + i);
        print_mac_byte(mac_byte);
        if(i < 5) PRINTS(":");
    }
    PRINTLN;

    outb(bar0 + 0x50, 0xc0); // Unlock chip to change protected settings

    // Prepare Receive Buffer and provide it to NIC for DMA
    uint64_t phys_addr = pmm_alloc_contiguous(4);
    rx_buffer = (uint8_t*)(phys_addr + hhdm_offset);
    for(uint64_t i = 0; i < 4 * 4096; i++) {
        rx_buffer[i] = 0;
    }
    outl(bar0 + 0x30, phys_addr); // Give hardware physical address

    outl(bar0 + 0x44, 0x8f);    // Filter rules
    outb(bar0 + 0x37, 0x0c);    // Enable use of RX bucket
    outb(bar0 + 0x50, 0x00);    // Lock config registers to prevent overwrites

    PRINTS(RX_BUFFER_LISTENING);
}

static uint32_t rx_read_ptr = 0;
static uint32_t bytes_left_in_packet = 0;
static uint8_t* current_data_ptr = 0; // Use a pointer instead of an offset

uint8_t read_ethernet() {
    while (bytes_left_in_packet == 0) {
        if (!(inb(global_bar0 + 0x37) & 0x01)) {
            
            uint8_t* packet_header = &rx_buffer[rx_read_ptr];
            uint16_t total_len = *(uint16_t*)(packet_header + 2);

            // Point exactly to the data (Skip 4 NIC + 14 Ethernet bytes)
            current_data_ptr = packet_header + 18;
            if(total_len < 18) 
                bytes_left_in_packet = 0;
            else    
                bytes_left_in_packet = total_len - 18;

            // Move pointer for NEXT packet
            rx_read_ptr = (rx_read_ptr + total_len + 4 + 3) & ~3;
            
            // Wrap around check for the 8KB ring buffer
            if(rx_read_ptr >= 8192) 
                rx_read_ptr -= 8192;

            outw(global_bar0 + 0x38, rx_read_ptr - 16);
        }
    }

    uint8_t data = *current_data_ptr;
    
    current_data_ptr++;
    bytes_left_in_packet--;

    return data;
}