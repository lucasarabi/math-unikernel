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

#define TILE_SIZE 64
void matrix_multiply(float* a, float* b, float* out, uint64_t n) {
    // Round n up to nearest multiple of 8 for AVX
    uint64_t n_padded = (n + 7) & ~7ULL;

    // Zero the output matrix (covers padded region too)
    for (uint64_t i = 0; i < n_padded * n_padded; i++)
        out[i] = 0.0f;

    for (uint64_t i = 0; i < n; i += TILE_SIZE) {
        for (uint64_t k = 0; k < n; k += TILE_SIZE) {
            for (uint64_t j = 0; j < n_padded; j += TILE_SIZE) {

                uint64_t i_end = i + TILE_SIZE < n        ? i + TILE_SIZE : n;
                uint64_t k_end = k + TILE_SIZE < n        ? k + TILE_SIZE : n;
                uint64_t j_end = j + TILE_SIZE < n_padded ? j + TILE_SIZE : n_padded;

                for (uint64_t ii = i; ii < i_end; ii++) {
                    for (uint64_t kk = k; kk < k_end; kk++) {
                        __m256 a_val = _mm256_set1_ps(a[ii * n + kk]);

                        for (uint64_t jj = j; jj < j_end; jj += 8) {
                            __m256 b_vec   = _mm256_load_ps(&b[kk * n_padded + jj]);
                            __m256 out_vec = _mm256_load_ps(&out[ii * n_padded + jj]);
                            out_vec = _mm256_fmadd_ps(a_val, b_vec, out_vec);
                            _mm256_store_ps(&out[ii * n_padded + jj], out_vec);
                        }
                    }
                }
            }
        }
    }
}