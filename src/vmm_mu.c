#include "headers/limine.h"
#include "headers/vmm_mu.h"
#include "headers/pmm_mu.h"
#include "headers/lib_mu.h"
#include "headers/hhdm_offset.h"
#include "headers/io_mu.h"

#define PRINTS write_serial_str
#define PRINTD write_serial_dec
#define PRINTH write_serial_hex
#define PRINTLN write_serial_str("\n");
#define PRINTF(str, val) PRINTS(str); PRINTS(" "); PRINTD(val);

#define PML4_SHIFT 39
#define PDPT_SHIFT 30
#define PD_SHIFT   21
#define PT_SHIFT   12

#define CR3_LOADED "VMM PML4 has been loaded to CR3 register.\n"

struct vmm_context vmm; 

static void vmm_prep_pml4() {
    vmm.pml4_phys = pmm_alloc();
    vmm.pml4_virt = (page_table_t*)(vmm.pml4_phys + hhdm_offset);
    memset(vmm.pml4_virt, 0, 4096);
}

void vmm_map_virt_to_phys(uint64_t virt_addr, uint64_t phys_addr, uint64_t flags) {
    page_table_t* current_table = vmm.pml4_virt;

    for(int level = 3; level > 0; level--) {
        uint64_t index = (virt_addr >> (12 + (level*9))) & VMM_INDEX_MASK; 

        if(!(current_table->entries[index] & VMM_PRESENT)) {
            uint64_t new_table_phys = pmm_alloc();
            page_table_t* new_table_virt = (page_table_t*)(new_table_phys + hhdm_offset);
            memset(new_table_virt, 0, 4096);

            current_table->entries[index] = new_table_phys | VMM_PRESENT | VMM_WRITEABLE;
        }
    
        current_table = (page_table_t*)(VMM_GET_ADDR(current_table->entries[index]) + hhdm_offset);
    }

    uint64_t pt_index = (virt_addr >> 12) & VMM_INDEX_MASK;
    current_table->entries[pt_index] = (phys_addr & VMM_ADDR_MASK) | flags | VMM_PRESENT;

    __asm__ volatile("invlpg (%0)" :: "r"(virt_addr) : "memory");
}

void vmm_map_range(uint64_t virt_start, uint64_t phys_start, uint64_t size, uint64_t flags) {
    for(uint64_t offset = 0; offset < size; offset += 4096) {
        vmm_map_virt_to_phys(virt_start + offset, phys_start + offset, flags);
    }
}

void vmm_init(struct limine_kernel_address_response* kernel_addr_response, struct limine_memmap_response* memmap_response) {
    vmm_prep_pml4();
    
    vmm_map_range(0x0, 0x0, 0x400000, VMM_PRESENT | VMM_WRITEABLE);

    uint64_t kernel_size = 0;
    for(uint64_t i = 0; i < memmap_response->entry_count; i++) {
        struct limine_memmap_entry* entry = memmap_response->entries[i];
        if(entry->type == LIMINE_MEMMAP_KERNEL_AND_MODULES) 
            kernel_size = entry->length;
    }

    vmm_map_range(kernel_addr_response->virtual_base, kernel_addr_response->physical_base, kernel_size, VMM_PRESENT | VMM_WRITEABLE);

    for(uint64_t i = 0; i < memmap_response->entry_count; i++) {
        struct limine_memmap_entry* entry = memmap_response->entries[i];
        if(entry->type == LIMINE_MEMMAP_USABLE || entry->type == LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE) {
            vmm_map_range(entry->base + hhdm_offset, entry->base, entry->length, VMM_PRESENT | VMM_WRITEABLE);
        }
    }
}

void vmm_activate() {
    __asm__ volatile ("mov %0, %%cr3" :: "r"(vmm.pml4_phys) : "memory");
    PRINTS(CR3_LOADED);
}

uint64_t next_virt_addr = 0xffff900000000000;
void* vmm_alloc(uint64_t num_pages, uint64_t flags) {
    void* start_addr = (void*)next_virt_addr;

    for(int i = 0; i < num_pages; i++) {
        uint64_t phys_addr = pmm_alloc();
        vmm_map_virt_to_phys(next_virt_addr, phys_addr, flags);
        memset((void*)next_virt_addr, 0, 4096);

        next_virt_addr += 4096;
    }
    
    return start_addr;
}
