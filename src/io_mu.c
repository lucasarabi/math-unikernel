#include "headers/io_mu.h"

#define COM1 0x3f8
#define LINE_STATUS_REG (COM1 + 5)

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %b0, %w1" :: "a"(val), "Nd"(port) : "memory");
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %w1, %b0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void serial_init() {
    outb(COM1+1, 0x00);
    outb(COM1+3, 0x80);
    outb(COM1+0, 0x01);
    outb(COM1+1, 0x00);
    outb(COM1+3, 0x03);
    outb(COM1+2, 0xc7);
}

static inline int is_transit_empty() {
    return inb(LINE_STATUS_REG) & 0x20;
}

static void write_serial(char a) {
    while(is_transit_empty() == 0);
    outb(COM1, a);
}

void write_serial_str(const char* s) {
    for(int i = 0; s[i] != '\0'; i++) {
        write_serial(s[i]);
    }
}

void write_serial_hex(uint64_t val) {
    char* hex_chars = "0123456789ABCDEF";

    write_serial_str("0x");
    for(int i = 15; i >= 0; i--) {
        uint8_t nibble = (val >> (i*4)) & 0xF;
        write_serial(hex_chars[nibble]);
    }
}

void write_serial_dec(uint64_t val) {
    if (val == 0) {
        write_serial('0');
        return;
    }

    char buffer[21]; 
    int i = 0;

    while (val > 0) {
        buffer[i++] = (val % 10) + '0';
        val /= 10;
    }

    for (int j = i - 1; j >= 0; j--) {
        write_serial(buffer[j]);
    }
}

void write_serial_bin(uint32_t val, uint8_t bits) {
    if (bits > 32) bits = 32; 

    write_serial_str("0b");

    for (int i = bits - 1; i >= 0; i--) {
        if (val & (1U << i)) {
            write_serial('1');
        } else {
            write_serial('0');
        }

        if (i > 0 && i % 4 == 0) {
            write_serial('_');
        }
    }
}
