#ifndef IO_MU_H
#define IO_MU_H

#include <stdint.h>

// I/O Serial Driver API
void serial_init();
void write_serial_str(const char* s);
void write_serial_hex(uint64_t val);
void write_serial_dec(uint64_t val);
void write_serial_bin(uint32_t val, uint8_t bits);

#define PRINTS write_serial_str
#define PRINTD write_serial_dec
#define PRINTH write_serial_hex
#define PRINTFLOAT(val) write_serial_float(val, 1)
#define PRINTLN write_serial_str("\n");
#define PRINTF(str, val) PRINTS(str); PRINTS(" "); PRINTD(val);

#endif
