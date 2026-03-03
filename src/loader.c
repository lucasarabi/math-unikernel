#include "headers/loader.h"
#include "headers/io.h"

#define MAGIC_NUMBER 0x474F2121 // "GO!!" ASCII values 

bool is_magic_number_received() {
    uint32_t current = 0;
    while(current != MAGIC_NUMBER) {
        uint8_t received = read_serial(); 
        current = current << 8;
        current |= received;
    }

    return true;
}
