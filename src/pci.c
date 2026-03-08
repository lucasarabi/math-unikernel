#include "headers/pci.h"
#include "headers/io.h"
#include "headers/display.h"

#define PCI_CONFIG_ADDRESS  0xcf8
#define PCI_CONFIG_DATA     0xcfc

// Vendor IDs for different NICs
#define INTEL_VENDOR_ID     0x8086      // Intel   (old hardware, currently emulated in QEMU setup)
#define KILLER_VENDOR_ID    0x1969      // Killer  (as found in my Dell G7)
#define REALTEK_VENDOR_ID   0x10ec      // Realtek (inside friend's gaming laptop, we will likely use this for showcase)

#define FOUND_INTEL_ID      "PCI: Found Intel NIC.\n"
#define FOUND_KILLER_ID     "PCI: Found Killer NIC.\n"
#define FOUND_REALTEK_ID     "PCI: Found Realtek NIC.\n"

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
                    uint32_t bar0 = pci_read_dword(bus, slot, 0, 0x10);

                    switch(vendor) {
                        case INTEL_VENDOR_ID:
                            PRINTS(FOUND_INTEL_ID);
                            // e1000_init(bus, slot, bar0);
                        break;

                        case KILLER_VENDOR_ID:
                            PRINTS(FOUND_KILLER_ID);
                            // killer_init(bus, slot, bar0);
                        break;

                        case REALTEK_VENDOR_ID:
                            PRINTS(FOUND_REALTEK_ID);
                            // rt8139_init(bus, slot, bar0);
                        break;
                    }

                    return PCI_INIT_SUCCESS;
                }
            }
        }
    }

    return 0;
}