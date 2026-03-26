#ifndef NETWORK_MU_H
#define NETWORK_MU_H

#include <stdint.h>

#define ETHERTYPE_CUSTOM 0x88B5

// Network layer API
void network_set_dest(uint8_t* dest, uint64_t expected_bytes);
void network_receive_frame(uint8_t* data, uint16_t length);
void network_set_poll_fn(void (*fn)());
void network_set_send_fn(void (*fn)(uint8_t* data, uint16_t length));
uint64_t network_bytes_received();
uint8_t read_ethernet();
void reset_header();

#endif