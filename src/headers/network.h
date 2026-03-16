#ifndef NETWORK_MU_H
#define NETWORK_MU_H

#include <stdint.h>

#define ETHERTYPE_CUSTOM 0x88B5

void network_receive_frame(uint8_t* data, uint16_t length);
uint8_t read_ethernet();

#endif