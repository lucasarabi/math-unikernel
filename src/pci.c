#include "headers/pci.h"
#include "headers/io.h"
#include "headers/display.h"

#define PCI_CONFIG_ADDRESS  0xcf8
#define PCI_CONFIG_DATA     0xcfc

// Vendor IDs for different NICs
#define INTEL_VENDOR_ID     0x8086      // Intel e1000  (old hardware, currently emulated in QEMU setup)
#define KILLER_VENDOR_ID    0x1969      // Killer E2400 (as found in my Dell G7)

#define START_SCAN      "PCI: Starting scan...\n"
#define SCAN_COMPLETE "PCI: Scan complete.\n"

uint32_t pci_read_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)
{
    uint32_t lbus = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;

    uint32_t address = (uint32_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));

    outl(PCI_CONFIG_ADDRESS, address);
    return inl(PCI_CONFIG_DATA);
}

uint16_t pci_scan_bus()
{
    for (uint16_t bus = 0; bus < 256; bus++)
    {
        for (uint8_t slot = 0; slot < 32; slot++)
        {
            uint32_t reg0 = pci_read_dword(bus, slot, 0, 0);
            uint16_t vendor = reg0 & 0xffff;

            if (vendor != 0xffff)
            {
                uint32_t reg3 = pci_read_dword(bus, slot, 0, 0x08);
                uint8_t base_class = (reg3 >> 24) & 0xff;
                uint8_t sub_class = (reg3 >> 16) & 0xff;

                if (base_class == 0x02 && sub_class == 0x00)
                {
                    if (vendor == INTEL_VENDOR_ID || vendor == KILLER_VENDOR_ID)
                    {
                        return PCI_INIT_SUCCESS;
                    }
                }
            }
        }
    }

    return 0;
}