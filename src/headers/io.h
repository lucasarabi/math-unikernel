#ifndef IO_MU_H
#define IO_MU_H

#include <stdint.h>

void serial_init();
void write_serial_str(const char* s);
void write_serial_hex(uint64_t val);
void write_serial_dec(uint64_t val);
void write_serial_bin(uint32_t val, uint8_t bits);

#endif
