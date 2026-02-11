#ifndef PMM_MU_H
#define PMM_MU_H

#include <stdint.h>
#include <stdbool.h>

struct limine_memmap_response;

struct pmm_context {
    uint8_t* bitmap;
    uint64_t bitmap_size;
    uint64_t total_frames;
    uint64_t free_frames;
};

extern struct pmm_context pmm;

// Physical Memory Manager API
void pmm_init(struct limine_memmap_response* response, uint64_t hhdm_offset);
uint64_t pmm_alloc();
void pmm_free(uint64_t phys_addr);

#endif
