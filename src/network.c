#include "headers/network.h"
#include "headers/display.h"
#include "nic_drivers/i219.h"

#define RX_BUF_SIZE 8192  // Large enough for any incoming frame

static uint8_t  rx_byte_buf[RX_BUF_SIZE];
static uint16_t rx_buf_len   = 0;  // How many bytes are currently in the buffer
static uint16_t rx_buf_read  = 0;  // How many bytes loader.c has consumed

// Called by i219_poll_rx() when a complete frame arrives.
// Strips the 14-byte Ethernet header and stores the payload.
void network_receive_frame(uint8_t* data, uint16_t length) {

    // Ethernet header is always 14 bytes:
    // [6 bytes: dst MAC] [6 bytes: src MAC] [2 bytes: EtherType]
    #define ETH_HEADER_SIZE 14

    if (length <= ETH_HEADER_SIZE) return;  // Runt frame, drop it

    // Check EtherType — bytes 12 and 13 of the frame
    uint16_t ethertype = ((uint16_t)data[12] << 8) | data[13];
    if (ethertype != ETHERTYPE_CUSTOM) return;  // Not our frame, drop it

    uint8_t*  payload     = data + ETH_HEADER_SIZE;
    uint16_t  payload_len = length - ETH_HEADER_SIZE;

    if (payload_len > RX_BUF_SIZE) {
        PRINTS("NET: Frame too large, dropping.\n");
        return;
    }

    // Copy payload into our internal buffer and reset the read cursor
    for (uint16_t i = 0; i < payload_len; i++)
        rx_byte_buf[i] = payload[i];

    rx_buf_len  = payload_len;
    rx_buf_read = 0;
}

// Called by loader.c one byte at a time.
// Blocks (spins) until a byte is available.
uint8_t read_ethernet() {
    while (rx_buf_read >= rx_buf_len) {
        // Buffer exhausted — wait for the next frame to arrive
        i219_poll_rx();
    }
    return rx_byte_buf[rx_buf_read++];
}