#pragma GCC target("avx,fma")

#include "headers/mathlib.h"
#include <immintrin.h>

float dot_product(float* a, float* b, uint64_t count) {
    __m256 sum_vec = _mm256_setzero_ps();

    for (uint64_t i = 0; i < count; i += 8) {
        __m256 va = _mm256_load_ps(&a[i]);
        __m256 vb = _mm256_load_ps(&b[i]);
        sum_vec = _mm256_fmadd_ps(va, vb, sum_vec);
    }

    float out[8] __attribute__((aligned(32)));
    _mm256_store_ps(out, sum_vec);

    float total = 0;
    for (int i = 0; i < 8; i++)
        total += out[i];

    return total;
}

void matrix_multiply(float* a, float* b, float* out, uint64_t n) {
    // implementation
}