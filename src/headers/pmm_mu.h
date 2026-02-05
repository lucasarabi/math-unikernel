#ifndef PMM_MU_H
#define PMM_MU_H

#include <stdint.h>

struct limine_memmap_response;

struct pmm_bitmap {
    uint8_t* bitmap;
    uint64_t bitmap_size;
    uint64_t total_frames;
    uint64_t free_frames;
};

// TO-DO -- expose bitmap by making it global variable
// extern struct pmm_bitmap mu_pmm;

extern struct pmm_bitmap pmm;

void pmm_init(struct limine_memmap_response* response, uint64_t hhdm_offset);

#endif
