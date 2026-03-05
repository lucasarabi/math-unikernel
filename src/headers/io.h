#ifndef IO_MU_H
#define IO_MU_H

#include <stdint.h>

// I/O Serial Driver API
void serial_init(uint32_t baud_rate);
uint8_t read_serial();

#endif
