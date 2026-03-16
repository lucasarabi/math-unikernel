#include "headers/pci.h"
#include "headers/io.h"
#include "headers/lib.h"
#include "headers/display.h"
#include "nic_drivers/rtl8139.h"
#include "nic_drivers/i219.h"

#define PCI_CONFIG_ADDRESS  0xcf8
#define PCI_CONFIG_DATA     0xcfc

// Vendor IDs for different NICs
#define INTEL_VENDOR_ID     0x8086      
#define REALTEK_VENDOR_ID   0x10ec   

#define FOUND_INTEL_ID      "PCI: Found Intel NIC.\n"
#define FOUND_REALTEK_ID    "PCI: Found Realtek NIC.\n"
#define WAKE_NIC            "PCI: Waking NIC from D3 sleep state.\n"

uint32_t pci_read_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t lbus = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;

    uint32_t address = (uint32_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));

    outl(PCI_CONFIG_ADDRESS, address);
    return inl(PCI_CONFIG_DATA);
}

void pci_write_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t data) {
    uint32_t lbus = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;

    uint32_t address = (uint32_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));

    outl(PCI_CONFIG_ADDRESS, address);
    outl(PCI_CONFIG_DATA, data);
}

void pci_wake_device(uint16_t bus, uint8_t slot, uint8_t func) {
    // The Status register is the upper 16 bits of offset 0x04.
    // Bit 4 of the Status register indicates if a capabilities list exists.
    uint16_t status = (pci_read_dword(bus, slot, func, 0x04) >> 16) & 0xFFFF;
    if (!(status & (1 << 4))) {
        return; // No capabilities list, hardware is likely always awake
    }

    // Get the starting pointer of the linked list (Offset 0x34)
    uint8_t cap_ptr = pci_read_dword(bus, slot, func, 0x34) & 0xFF;

    // Walk the linked list
    while (cap_ptr != 0) {
        // Read the Capability Header
        // Byte 0 = Capability ID, Byte 1 = Pointer to the next capability
        uint32_t cap_header = pci_read_dword(bus, slot, func, cap_ptr);
        uint8_t cap_id = cap_header & 0xFF;
        uint8_t next_ptr = (cap_header >> 8) & 0xFF;

        // ID 0x01 is the official PCI Power Management capability
        if (cap_id == 0x01) { 
            // The Power Management Control/Status Register (PMCSR) is at offset + 4
            uint32_t pmcsr = pci_read_dword(bus, slot, func, cap_ptr + 4);

            // The current power state is stored in the lowest 2 bits.
            // 00 = D0 (Awake), 11 = D3 (Asleep)
            if ((pmcsr & 0x03) != 0) {
                PRINTS(WAKE_NIC);
                
                // Clear the lowest 2 bits to 0 to force the D0 state
                pmcsr &= ~0x03;
                pci_write_dword(bus, slot, func, cap_ptr + 4, pmcsr);
                
                // ~10ms delay 
                for(volatile uint32_t i = 0; i < 10000000; i++) { 
                    __asm__("nop"); 
                }
            }
            break; 
        }
        
        cap_ptr = next_ptr; 
    }
}

uint16_t pci_scan_bus() {
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t slot = 0; slot < 32; slot++) {
            for (uint8_t func = 0; func < 8; func++) {
                uint32_t reg0 = pci_read_dword(bus, slot, func, 0);
                uint16_t vendor = reg0 & 0xffff;
                uint16_t device_id = (reg0 >> 16) & 0xffff;
                // If vendor is 0xFFFF, this specific function doesn't exist. 
                if (vendor == 0xffff) 
                    continue;

                uint32_t reg3 = pci_read_dword(bus, slot, func, 0x08);
                uint8_t base_class = (reg3 >> 24) & 0xff;
                uint8_t sub_class = (reg3 >> 16) & 0xff;

                // If not a Network controller or Ethernet controller, continue.
                if (base_class != 0x02 || sub_class != 0x00)
                    continue;

                // Get Base Address Register for NIC
                uint32_t bar0 = pci_read_dword(bus, slot, func, 0x10);

                pci_wake_device(bus, slot, func);

                // Enable bus mastering and MMIO
                uint32_t command_reg = pci_read_dword(bus, slot, func, 0x04);
                command_reg |= (1<<1);
                command_reg |= (1<<2);
                pci_write_dword(bus, slot, func, 0x04, command_reg);

                uint8_t irq_line = pci_read_dword(bus, slot, func, 0x3c) & 0xff;

                switch (vendor)
                {
                case INTEL_VENDOR_ID:
                    // PRINTS(FOUND_INTEL_ID);
                    PRINTS("NIC: Vendor: "); PRINTH(vendor); PRINTLN;
                    PRINTS("NIC: Device ID: "); PRINTH(device_id); PRINTLN;
                    i219_init(bar0, irq_line);
                    break;

                case REALTEK_VENDOR_ID:
                    // PRINTS(FOUND_REALTEK_ID);
                    PRINTS("NIC: Vendor: "); PRINTH(vendor); PRINTLN;
                    PRINTS("NIC: Device ID: "); PRINTH(device_id); PRINTLN;
                    rtl8139_init(bar0);
                    break;
                }

                return PCI_INIT_SUCCESS;
            }
        }
    }
    return 0;
}