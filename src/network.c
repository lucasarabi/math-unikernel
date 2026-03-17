#include "headers/network.h"
#include "headers/display.h"
#include "nic_drivers/i219.h"

#define ETH_HEADER_SIZE 14
#define HEADER_FRAME_SIZE 16                    // 4 bytes magic + 8 bytes size + some slack

static uint8_t* rx_dest = 0;                    // Destination buffer (huge page)
static uint64_t rx_expected = 0;                // Total bytes we expect to receive
static uint64_t rx_received = 0;                // Total bytes received so far
static uint8_t rx_ready = 0;                    // 1 when a complete byte is available for read_ethernet()

static uint8_t hdr_buf[HEADER_FRAME_SIZE];
static uint16_t hdr_len = 0;
static uint16_t hdr_read = 0;
static uint8_t hdr_done = 0;                    // 1 once magic + size have been consumed

void network_set_dest(uint8_t* dest, uint64_t expected_bytes) {
    rx_dest     = dest;
    rx_expected = expected_bytes;
    rx_received = 0;
    hdr_done    = 0;
    hdr_len     = 0;
    hdr_read    = 0;
}

void network_receive_frame(uint8_t* data, uint16_t length) {
    if (length <= ETH_HEADER_SIZE) return;

    uint16_t ethertype = ((uint16_t)data[12] << 8) | data[13];
    if (ethertype != ETHERTYPE_CUSTOM) return;

    uint8_t*  payload     = data + ETH_HEADER_SIZE;
    uint16_t  payload_len = length - ETH_HEADER_SIZE;

    if (!hdr_done) {
        // First frame, contains magic + size header
        // Copy into header buffer for read_ethernet() to consume byte by byte
        if (payload_len > HEADER_FRAME_SIZE) payload_len = HEADER_FRAME_SIZE;
        for (uint16_t i = 0; i < payload_len; i++)
            hdr_buf[i] = payload[i];
        hdr_len  = payload_len;
        hdr_read = 0;
        rx_ready = 1;
        return;
    }

    // Subsequent frames, append directly into the huge page
    if (!rx_dest) return;

    for (uint16_t i = 0; i < payload_len; i++) {
        if (rx_received >= rx_expected) break;
        rx_dest[rx_received++] = payload[i];
    }

    if (rx_received >= rx_expected)
        PRINTS("NET: Transfer complete.\n");
}

uint8_t read_ethernet() {
    // Drain the header buffer (magic + size)
    if (!hdr_done) {
        while (!rx_ready || hdr_read >= hdr_len)
            __asm__ volatile("hlt");

        uint8_t byte = hdr_buf[hdr_read++];

        // mark header done so subsequent frames go into the huge page
        if (hdr_read >= 12)
            hdr_done = 1;

        return byte;
    }

    // Block until the next byte lands in the huge page
    while (rx_received <= /* bytes consumed so far */ 0)
        __asm__ volatile("hlt");

    return 0;
}

// Returns how many payload bytes have arrived so far.
uint64_t network_bytes_received() {
    return rx_received;
}