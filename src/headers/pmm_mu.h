#ifndef PMM_MU_H
#define PMM_MU_H

#include <stdint.h>

struct limine_memmap_response;

struct pmm_bitmap {
    uint8_t* bitmap;
    uint64_t bitmap_size;
};

// TO-DO -- expose bitmap by making it global variable
// extern struct pmm_bitmap mu_pmm;

void pmm_init(struct limine_memmap_response* response);

#endif
