#ifndef NETWORK_MU_H
#define NETWORK_MU_H

#include <stdint.h>

#define ETHERTYPE_CUSTOM 0x88B5

// Network layer API
void network_receive_frame(uint8_t* data, uint16_t length);
uint8_t read_ethernet();
void network_set_poll_fn(void (*fn)());

#endif