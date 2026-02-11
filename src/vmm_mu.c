#include "headers/vmm_mu.h"
#include "headers/pmm_mu.h"
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
#define VMM_INDEX_MASK  0x1FF 

#define VMM_PRESENT         (1ULL << 0)
#define VMM_WRITEABLE       (1ULL << 1)
#define VMM_CACHE_DISABLE   (1ULL << 4)
#define VMM_ACCESSED        (1ULL << 5)
#define VMM_DIRTY           (1ULL << 6)
#define VMM_HUGE            (1ULL << 7)
#define VMM_NO_EXEC         (1ULL << 63)

#define VMM_ADDR_MASK  0x000FFFFFFFFFF000ULL
#define VMM_GET_ADDR(entry) ((entry) & VMM_ADDR_MASK)

// TODO implement page walk
/*
    Offset by 12.

    Shift by 39, mask 9 bottom bits     PML4
    Shift by 30, mask 9 bottom bits     PDPT
    Shift by 21, mask 9 bottom bits     PD
    Shift by 12, mask 9 bottom bits     PT

    
    Shifting RIGHT will give the binary of the exact value to process. 
    Compare it to 0x1ff (= 0b111111111 = 511)
*/


struct vmm_context vmm; 

void page_walk(uint64_t virt_addr) {

    page_table_t* current_table = vmm.pml4_virt;

    for(int level = 3; level >= 0; level--) {
        uint64_t index = (virt_addr >> (12 + (level*9))) & VMM_INDEX_MASK; 

        pt_entry_t entry = current_table->entries[index];
        
        if(level > 0) {
            if(!(entry & VMM_PRESENT)) {
                entry = pmm_alloc();

                // TODO continue page walk

            }
        }
    }

}