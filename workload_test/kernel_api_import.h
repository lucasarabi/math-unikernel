#ifndef KERNEL_API_HEADER_MU_H
#define KERNEL_API_HEADER_MU_H

#include <stdint.h>

#define KERNEL_API_ADDRESS 0x10000000

#define MAIN __attribute__((section(".entry")))

#define VMM_PRESENT         (1ULL << 0)
#define VMM_WRITEABLE       (1ULL << 1)

typedef struct {
    void*  (*alloc_huge_page)(uint64_t n, uint64_t flags);
    float  (*dot_product)(float* a, float* b, uint64_t count);
    void   (*matrix_multiply)(float* a, float* b, float* out, uint64_t n);
    void   (*printd)(uint64_t);
    void   (*prints)(const char*);
    void   (*set_output)(void* buffer, uint64_t size);
    void*  output_buffer;
    uint64_t output_size;
} kernel_api_t;

#endif