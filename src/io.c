#include "headers/io.h"

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

uint8_t serial_init(uint32_t baud_rate) {
    uint16_t divisor = 115200 / baud_rate;
    uint8_t divisor_lo = divisor & 0xff;
    uint8_t divisor_hi = (divisor >> 8) & 0xff;

    // Configure UART chip
    outb(COM1+1, 0x00);     // disable interrupts
    outb(COM1+3, 0x80);     // Enable Divisor Latch Access Bit
    // Set baud rate
    outb(COM1+0, divisor_lo);
    outb(COM1+1, divisor_hi);
    // Lock DLAB and set line mode
    outb(COM1+3, 0x03);     // 8 bits, no parity, one stop bit (standard 8N1)
    outb(COM1+2, 0xc7);     // enable FIFO, clear them, with 14-byte threshold
    outb(COM1+4, 0x0b);     // IRQs enabled, RTS/DSR set

    return SERIAL_INIT_SUCCESS;
}

/*
static inline int is_transit_empty() {
    return inb(LINE_STATUS_REG) & 0x20;
}

static void write_serial(char a) {
    while(is_transit_empty() == 0);
    outb(COM1, a);
}
*/

static inline int is_data_ready() {
    return inb(LINE_STATUS_REG) & 0x01;
}

uint8_t read_serial() {
    while(is_data_ready() == 0);
    return inb(COM1);
}
