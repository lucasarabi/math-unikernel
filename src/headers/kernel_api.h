#ifndef KERNEL_API_MU_H
#define KERNEL_API_MU_H

#include <stdint.h>

#define KERNEL_API_ADDRESS 0x10000000

typedef struct {
    void* (*alloc_huge_page)(uint64_t n, uint64_t flags);
    void* output_buffer;
    uint64_t output_size;
} kernel_api_t;

#endif