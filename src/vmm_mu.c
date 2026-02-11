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

void vmm_map_virt_to_phys(uint64_t virt_addr, uint64_t phys_addr, uint64_t flags) {

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

void test_vmm_logic() {
    PRINTS("--- Starting VMM Surveyor Test ---\n");

    // 1. Define a test mapping: Virtual 0xDEADBEEF000 -> Physical 0x123456000
    uint64_t test_virt = 0xDEADBEEF000;
    uint64_t test_phys = 0x123456000;
    
    PRINTS("Mapping 0xDEADBEEF000 to 0x123456000...\n");
    vmm_map_virt_to_phys(test_virt, test_phys, VMM_WRITEABLE | VMM_PRESENT);

    // 2. Manual Verification (Walking the tree outside the function)
    page_table_t* table = vmm.pml4_virt;
    
    // Level 3 (PML4)
    uint64_t i3 = (test_virt >> 39) & VMM_INDEX_MASK;
    PRINTS("Checking PML4 Index "); PRINTD(i3); PRINTS(": ");
    if (table->entries[i3] & VMM_PRESENT) {
        PRINTS("OK (Phys: "); PRINTH(VMM_GET_ADDR(table->entries[i3])); PRINTS(")\n");
        table = (page_table_t*)(VMM_GET_ADDR(table->entries[i3]) + hhdm_offset);
    } else {
        PRINTS("FAILED - Entry not present!\n"); return;
    }

    // Level 2 (PDPT)
    uint64_t i2 = (test_virt >> 30) & VMM_INDEX_MASK;
    PRINTS("Checking PDPT Index "); PRINTD(i2); PRINTS(": ");
    if (table->entries[i2] & VMM_PRESENT) {
        PRINTS("OK\n");
        table = (page_table_t*)(VMM_GET_ADDR(table->entries[i2]) + hhdm_offset);
    } else {
        PRINTS("FAILED!\n"); return;
    }

    // Level 1 (PD)
    uint64_t i1 = (test_virt >> 21) & VMM_INDEX_MASK;
    PRINTS("Checking PD Index "); PRINTD(i1); PRINTS(": ");
    if (table->entries[i1] & VMM_PRESENT) {
        PRINTS("OK\n");
        table = (page_table_t*)(VMM_GET_ADDR(table->entries[i1]) + hhdm_offset);
    } else {
        PRINTS("FAILED!\n"); return;
    }

    // Level 0 (PT) - THE LEAF
    uint64_t i0 = (test_virt >> 12) & VMM_INDEX_MASK;
    PRINTS("Checking PT Index "); PRINTD(i0); PRINTS(": ");
    uint64_t final_entry = table->entries[i0];
    if ((final_entry & VMM_PRESENT) && (VMM_GET_ADDR(final_entry) == test_phys)) {
        PRINTS("SUCCESS! Physical address matched: "); PRINTH(VMM_GET_ADDR(final_entry)); PRINTS("\n");
    } else {
        PRINTS("FAILED - Destination mismatch or not present!\n");
    }

    PRINTS("--- VMM Test Complete ---\n");
}