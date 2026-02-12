#ifndef VMM_MU_H
#define VMM_MU_H

#include <stdint.h>

#define VMM_PRESENT         (1ULL << 0)
#define VMM_WRITEABLE       (1ULL << 1)
#define VMM_USER            (1ULL << 2) 
#define VMM_WRITE_THRU      (1ULL << 3) 
#define VMM_CACHE_DISABLE   (1ULL << 4) 
#define VMM_ACCESSED        (1ULL << 5)
#define VMM_DIRTY           (1ULL << 6)
#define VMM_HUGE            (1ULL << 7) 
#define VMM_GLOBAL          (1ULL << 8)
#define VMM_NO_EXEC         (1ULL << 63) 

#define VMM_ADDR_MASK   0x000FFFFFFFFFF000ULL
#define VMM_INDEX_MASK  0x1FF 
#define VMM_GET_ADDR(entry) ((entry) & VMM_ADDR_MASK)

typedef uint64_t pt_entry_t; // Stores phys addr of next table

typedef struct {
    pt_entry_t entries[512];                        
} __attribute__((aligned(4096))) page_table_t;

struct vmm_context {
    page_table_t* pml4_virt;    // PML4 table
    uintptr_t pml4_phys;        // To be loaded into CR3 register
};

extern struct vmm_context vmm;

void vmm_init();
void vmm_map_virt_to_phys(uint64_t virt_addr, uint64_t phys_addr, uint64_t flags);
void test_vmm_logic();

#endif
