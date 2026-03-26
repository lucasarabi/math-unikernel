#include "headers/loader.h"
#include "headers/network.h"          

#define MAGIC_NUMBER 0x474F2121        // "GO!!" — must match Python script

uint32_t unlock() {
    uint32_t magic_number = 0;
    do {
        uint8_t received = read_ethernet();
        magic_number <<= 8;
        magic_number |= received;
    } while(magic_number != MAGIC_NUMBER);
    return magic_number;
}

// Polls for a 64-bit integer. Bytes arrive least significant first (little-endian).
uint64_t poll_payload_size() {
    uint64_t number_of_bytes = 0;
    for (int i = 0; i < 8; i++) {
        uint64_t received = (uint64_t)read_ethernet();
        number_of_bytes |= received << (i * 8);
    }
    return number_of_bytes;
}

void poll_payload(uint8_t* payload_mem_addr, uint64_t payload_byte_size) {
    network_set_dest(payload_mem_addr, payload_byte_size);

    while (network_bytes_received() < payload_byte_size) {
        __asm__ volatile("hlt");
    }
}
