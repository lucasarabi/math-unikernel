#pragma GCC target("avx,fma")

#include <stdint.h>
#include <immintrin.h>
#include "kernel_api_import.h"

MAIN void run() {
    kernel_api_t* api = (kernel_api_t*)KERNEL_API_ADDRESS;

    float* vec_a = api->alloc_huge_page(1, VMM_PRESENT | VMM_WRITEABLE);
    float* vec_b = api->alloc_huge_page(1, VMM_PRESENT | VMM_WRITEABLE);

    uint64_t count = 524288;
    for (uint64_t i = 0; i < count; i++) {
        vec_a[i] = 1.0f;
        vec_b[i] = 2.0f;
    }

    float result = api->dot_product(vec_a, vec_b, count);

    float* output = api->alloc_huge_page(1, VMM_PRESENT | VMM_WRITEABLE);
    output[0] = result;
}