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

#define PML4_SHIFT      39
#define PDPT_SHIFT      30
#define PD_SHIFT        21
#define PT_SHIFT        12

struct vmm_context vmm; 

void vmm_init() {
    vmm.pml4_phys = pmm_alloc();
    vmm.pml4_virt = (page_table_t*)(vmm.pml4_phys + hhdm_offset);
    memset(vmm.pml4_virt, 0, 4096);
}

void vmm_map_virt_to_phys(uint64_t virt_addr, uint64_t phys_addr, uint64_t flags) {

    // Sanity check -- check if PLM4 is initialized
    PRINTS("Checking root pointer: "); 
    PRINTH((uint64_t)vmm.pml4_virt); PRINTLN;
    if(vmm.pml4_virt == 0) {
       PRINTS("(ERROR) vmm.pml4_virt is NULL!\n");
       return;
    }

    page_table_t* current_table = vmm.pml4_virt;

    for(int level = 3; level >= 0; level--) {
        uint64_t index = (virt_addr >> (12 + (level*9))) & VMM_INDEX_MASK; 
        pt_entry_t entry = current_table->entries[index];

        // If NOT leaf node (PML4, PDPT, PD)
        if(level > 0) {
            // If not present, create table
            if(!(entry & VMM_PRESENT)) {
                uint64_t new_table_phys = pmm_alloc();

                // Zero out virtual page
                page_table_t* new_table_virt = (page_table_t*)(new_table_phys + hhdm_offset);
                memset(new_table_virt, 0 , 4096);
            
                // Add new phys table address to current table map
                current_table->entries[index] = new_table_phys | VMM_PRESENT | VMM_WRITEABLE;
                
                // Table is now present. Switch entry to new phys table addr before shifting down table
                entry = current_table->entries[index];

            }
            // Shift current table down
            current_table = (page_table_t*)(VMM_GET_ADDR(entry) + hhdm_offset);
        } 
        // leaf node (PT)
        else {
            current_table->entries[index] = (phys_addr & VMM_ADDR_MASK) | flags | VMM_PRESENT;
        }
    }
}
