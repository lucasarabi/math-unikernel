#ifndef IO_MU_H
#define IO_MU_H

#include <stdint.h>
#define SERIAL_INIT_SUCCESS     (1<<7)

// I/O Serial Driver API
uint8_t serial_init(uint32_t baud_rate);
uint8_t read_serial();

#endif
