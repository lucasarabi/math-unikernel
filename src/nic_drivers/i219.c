#include "i219.h"
#include "../headers/io.h"
#include "../headers/display.h"
#include "../headers/hhdm_offset.h"
#include "../headers/pmm.h"
#include "../headers/vmm.h"
#include "../headers/pic.h"
#include "../headers/idt.h"
#include "../headers/network.h"

#define MMIO_MAPPED         "NIC: I219-LM MMIO mapped.\n"
#define RESET_COMPLETE      "NIC: Reset complete.\n"
#define LINK_UP             "NIC: Link is up.\n"
#define LINK_DOWN           "NIC: WARNING: Link is down. Is the cable plugged in?\n"
#define MAC_ADDRESS         "NIC: MAC Address: "
#define HANDSHAKE_COMPLETE  "NIC: I219-LM handshake complete.\n"

#define KB              1024
#define NUM_RX_DESC     32
#define RX_BUF_SIZE     2048   // Must match RCTL.BSIZE setting below

#define CTRL      0x0000  // Device Control
#define STATUS    0x0008  // Device Status (read-only)
#define CTRL_EXT  0x0018  // Extended Device Control
#define RAL0      0x5400  // Receive Address Low  (MAC bytes 0-3)
#define RAH0      0x5404  // Receive Address High (MAC bytes 4-5)
#define RDBAL     0x2800  // RX Descriptor Base Address Low
#define RDBAH     0x2804  // RX Descriptor Base Address High
#define RDLEN     0x2808  // RX Descriptor Ring Length (bytes)
#define RDH       0x2810  // RX Descriptor Head (owned by hardware)
#define RDT       0x2818  // RX Descriptor Tail (owned by software)
#define RCTL      0x0100  // Receive Control
#define IMS       0x00D0  // Interrupt Mask Set
#define ICR       0x00C0  // Interrupt Cause Read (reading clears it)

#define CTRL_RST        (1 << 26)  // Software reset
#define CTRL_SLU        (1 << 6)   // Set Link Up — required on I219
#define CTRL_EXT_DRST   (1 << 29)  // PCH reset — must be issued on I219 before CTRL_RST

#define RCTL_EN         (1 << 1)   // Receiver Enable
#define RCTL_BAM        (1 << 15)  // Broadcast Accept Mode
#define RCTL_BSIZE_2048 (0 << 16)  // Buffer size = 2048 bytes (default)
#define RCTL_SECRC      (1 << 26)  // Strip Ethernet CRC from received frames

#define IMS_RXT0        (1 << 7)   // Interrupt on receive descriptor done

#define STATUS_LU       (1 << 1)   // Link Up bit in STATUS register

static volatile uint64_t mmio_base_virt;
static volatile struct i219_rx_descriptor* rx_ring_virt;
static uint8_t* rx_buffers_virt[NUM_RX_DESC];
static uint8_t rx_tail = 0;  // Software's current position in the ring

static inline uint32_t reg_read(uint32_t offset) {
    return *(volatile uint32_t*)(mmio_base_virt + offset);
}

static inline void reg_write(uint32_t offset, uint32_t val) {
    *(volatile uint32_t*)(mmio_base_virt + offset) = val;
}

static void print_mac_byte(uint8_t byte) {
    const char hex[] = "0123456789ABCDEF";
    char str[3];
    str[0] = hex[(byte >> 4) & 0x0F];
    str[1] = hex[byte & 0x0F];
    str[2] = '\0';
    PRINTS(str);
}

void i219_interrupt_handler();

void i219_init(uint32_t bar0_raw, uint8_t irq) {
    // Strip the low 4 flag bits from the BAR to get the physical base address.
    uint64_t mmio_base_phys = (uint64_t)bar0_raw & ~0xFULL;
    mmio_base_virt = mmio_base_phys + hhdm_offset;
    vmm_map_range(mmio_base_virt, mmio_base_phys, 128 * KB,
                  VMM_PRESENT | VMM_WRITEABLE | VMM_CACHE_DISABLE);
    PRINTS(MMIO_MAPPED);

    // Mask all interrupts before reset
    reg_write(0x00D8, 0xFFFFFFFF);

    // MAC reset only — I219-LM PCH side resets automatically
    reg_write(CTRL, reg_read(CTRL) | CTRL_RST);

    // Brief delay before polling — I219 needs a moment to begin reset
    for (volatile uint32_t i = 0; i < 1000; i++)
        __asm__("nop");
    while (reg_read(CTRL) & CTRL_RST);

    // Re-mask interrupts — reset clears them
    reg_write(0x00D8, 0xFFFFFFFF);

    PRINTS(RESET_COMPLETE);

    // The I219 requires software to explicitly assert SLU after reset.
    // Without this the PHY stays down and you receive nothing.
    reg_write(CTRL, reg_read(CTRL) | CTRL_SLU);

    if (reg_read(STATUS) & STATUS_LU)
        PRINTS(LINK_UP);
    else
        PRINTS(LINK_DOWN);

    uint32_t mac_lo = reg_read(RAL0);
    uint32_t mac_hi = reg_read(RAH0);

    uint8_t mac[6];
    mac[0] = mac_lo & 0xFF;
    mac[1] = (mac_lo >> 8)  & 0xFF;
    mac[2] = (mac_lo >> 16) & 0xFF;
    mac[3] = (mac_lo >> 24) & 0xFF;
    mac[4] = mac_hi & 0xFF;
    mac[5] = (mac_hi >> 8)  & 0xFF;

    PRINTS(MAC_ADDRESS);
    for (uint8_t i = 0; i < 6; i++) {
        print_mac_byte(mac[i]);
        if (i < 5) PRINTS(":");
    }
    PRINTLN;

    uint64_t rx_ring_phys = pmm_alloc();
    rx_ring_virt = (struct i219_rx_descriptor*)(rx_ring_phys + hhdm_offset);

    for (uint8_t i = 0; i < NUM_RX_DESC; i++) {
        uint64_t buf_phys = pmm_alloc();
        rx_buffers_virt[i] = (uint8_t*)(buf_phys + hhdm_offset);
        rx_ring_virt[i].addr   = buf_phys;
        rx_ring_virt[i].status = 0;
    }

    reg_write(RDBAL, (uint32_t)(rx_ring_phys & 0xFFFFFFFF));
    reg_write(RDBAH, (uint32_t)(rx_ring_phys >> 32));
    reg_write(RDLEN, NUM_RX_DESC * sizeof(struct i219_rx_descriptor));
    reg_write(RDH, 0);
    reg_write(RDT, NUM_RX_DESC - 1);

    reg_write(RCTL, RCTL_EN | RCTL_BAM | RCTL_BSIZE_2048 | RCTL_SECRC);

    reg_write(IMS, IMS_RXT0);
    pic_unmask(irq);
    register_irq_handler(irq, i219_interrupt_handler);

    PRINTS(HANDSHAKE_COMPLETE);
}

void i219_interrupt_handler() {
    // Reading ICR clears all pending interrupt bits in hardware.
    // If you don't read it the NIC will re-assert the IRQ immediately
    // after EOI and your CPU will spin in the handler forever.
    uint32_t icr = reg_read(ICR);
    (void)icr;

    // Drain any packets the NIC wrote while the interrupt was pending
    i219_poll_rx();
}

void i219_poll_rx() {
    // Walk the ring from our current tail position.
    // The NIC sets the DD (Descriptor Done) bit in status when it writes a packet.
    while (rx_ring_virt[rx_tail].status & 0x01) {

        uint8_t* packet = rx_buffers_virt[rx_tail];
        uint16_t length = rx_ring_virt[rx_tail].length;
        uint8_t  errors = rx_ring_virt[rx_tail].errors;

        if (errors) {
            PRINTS("NIC: Packet received with errors, dropping.\n");
        } else {
            network_receive_frame(packet, length);
        }

        // Clear the status so we can detect the next packet in this slot.
        rx_ring_virt[rx_tail].status = 0;

        // Advance our tail and tell the NIC it can reuse this descriptor.
        rx_tail = (rx_tail + 1) % NUM_RX_DESC;
        reg_write(RDT, rx_tail);
    }
}