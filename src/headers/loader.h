#ifndef LOADER_MU_H
#define LOADER_MU_H

#include <stdint.h>
#include <stdbool.h>

// Loader API
uint32_t unlock();
uint64_t poll_payload_size();
void poll_payload(uint8_t* payload_mem_addr, uint64_t payload_byte_size);

#endif
