#ifndef PMM_MU_H
#define PMM_MU_H

#include <stdint.h>

struct limine_memmap_response;

struct pmm_bitmap {
    uint8_t* bitmap;
    uint64_t bitmap_size;
    uint64_t total_frames;
    uint64_t total_free_frames;
    uint64_t total_claimed_frames;
};

// TO-DO -- expose bitmap by making it global variable
// extern struct pmm_bitmap mu_pmm;

extern struct pmm_bitmap pmm;

void pmm_init(struct limine_memmap_response* response);
void update_pmm_frame_count();

#endif
