#ifndef PMM_MU_H
#define PMM_MU_H

#include <stdint.h>
#include <stdbool.h>
#define PMM_INIT_SUCCESS    (1<<3)

struct limine_memmap_response;

struct pmm_context {
    uint8_t* bitmap;
    uint64_t bitmap_size;
    uint64_t total_frames;
    uint64_t free_frames;
};

extern struct pmm_context pmm;

// Physical Memory Manager API
uint8_t pmm_init(struct limine_memmap_response* response);
uint64_t pmm_alloc();
void pmm_free(uint64_t phys_addr);

// 2mb page API
uint64_t pmm_alloc_2mb();
void pmm_free_2mb(uint64_t phys_addr);

#endif
