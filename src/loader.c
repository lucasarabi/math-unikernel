#include "headers/loader.h"
#include "headers/io.h"

#define MAGIC_NUMBER 0x474F2121

void wait_for_magic_number() {
    uint32_t current = 0;
    while(current != MAGIC_NUMBER) {
        uint8_t received = read_serial(); 
        current = current << 8;
        current |= received;
    }

    PRINTS("Magic Number received!"); PRINTLN;
}
