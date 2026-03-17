#ifndef KERNEL_API_MU_H
#define KERNEL_API_MU_H

#include <stdint.h>

#define KERNEL_API_ADDRESS 0x10000000

typedef struct {
    // Memory
    void*  (*alloc_huge_page)(uint64_t n, uint64_t flags);

    // Math library
    float  (*dot_product)(float* a, float* b, uint64_t count);
    void   (*matrix_multiply)(float* a, float* b, float* out, uint64_t n);

    // Output, filled by workload before returning
    void*    output_buffer;
    uint64_t output_size; 

} kernel_api_t;

#endif