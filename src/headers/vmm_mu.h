#ifndef VMM_MU_H
#define VMM_MU_H

#include <stdint.h>

typedef uint64_t pt_entry_t;                        // Stores phys addr of next table

typedef struct {
    pt_entry_t entries[512];                        
} __attribute__((aligned(4096))) page_table_t;

struct vmm_context {
    page_table_t* pml4_virt;                        // PML4 table
    uintptr_t pml4_phys;                            // To be loaded into CR3 register
};

extern struct vmm_context vmm;

#endif
