#include "headers/loader.h"
#include "headers/io.h"

#define MAGIC_NUMBER 0x474F2121 // "GO!!" ASCII values 

uint32_t unlock() {
    uint32_t magic_number = 0;
    do {
        uint8_t received = read_serial(); 
        magic_number <<= 8;
        magic_number |= received;
    } while(magic_number != MAGIC_NUMBER);
    return magic_number;
}

// Polls for 64 bit integer. Bytes must be send most sigificant to least. 
uint64_t poll_payload_size() {
    uint64_t number_of_bytes = 0;
    for(int i = 0; i < 8; i++) {
        uint64_t received = (uint64_t)read_serial();
        number_of_bytes |= received << (i*8);
    }
    return number_of_bytes;
}

void poll_payload(uint8_t* payload_mem_addr, uint64_t payload_byte_size) {
    for(uint64_t i = 0; i < payload_byte_size; i++) {
        payload_mem_addr[i] = read_serial();
    }
}