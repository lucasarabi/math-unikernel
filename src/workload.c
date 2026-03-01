#pragma GCC target("avx")
#include "headers/workload.h"
#include "headers/io.h"
#include <immintrin.h>

float matrix_a[8] __attribute__((aligned(32))) = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
float matrix_b[8] __attribute__((aligned(32))) = {1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8};
float result[8]   __attribute__((aligned(32)));

void run_workload() {
    // 1. Load: Pull 8 floats from RAM into YMM registers
    __m256 ymm_a = _mm256_load_ps(matrix_a);
    __m256 ymm_b = _mm256_load_ps(matrix_b);

    // 2. Compute: Add all 8 pairs in one single CPU clock cycle [cite: 15, 52]
    __m256 ymm_res = _mm256_add_ps(ymm_a, ymm_b);

    // 3. Store: Push the result back into the result array in RAM
    _mm256_store_ps(result, ymm_res);

    for(uint8_t i = 0; i < 8; i++) {
        PRINTF("Index", i); PRINTS(": "); PRINTD(result[i]); PRINTLN;
    }
}