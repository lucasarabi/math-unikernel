// Crucial compiler directive -- utilize AVX features in this file that are otherwise disabled in the rest of the project
#pragma GCC target("avx,fma")  

#include "headers/workload.h"
#include "headers/vmm.h"
#include "headers/io.h"
#include <immintrin.h>

/* * HEAVY WORKLOAD
 * Processes 2MB of floats (524,288 elements) using Fused Multiply-Add.
 * Requires: AVX and FMA hardware support.
 */
static float run_heavy_workload(float* vec_a, float* vec_b) {
    uint32_t count = 524288; // Total floats in one 2MB Huge Page
    
    // YMM register to hold 8 partial sums
    __m256 sum_vec = _mm256_setzero_ps();

    for (uint32_t i = 0; i < count; i += 8) {
        // Load 8 floats from each huge page
        // Note: These MUST be 32-byte aligned (vmm_alloc_huge_pages handles this!)
        __m256 a = _mm256_load_ps(&vec_a[i]);
        __m256 b = _mm256_load_ps(&vec_b[i]);

        // sum = (a * b) + sum
        sum_vec = _mm256_fmadd_ps(a, b, sum_vec);
    }

    // Collapse the 8 partial sums into one final float
    float out[8] __attribute__((aligned(32)));
    _mm256_store_ps(out, sum_vec);
    
    float total_sum = 0;
    for(int i = 0; i < 8; i++) {
        total_sum += out[i];
    }

    return total_sum;
}

void run()
{
    PRINTS("Testing Heavy Workload...\n");

    // 1. Allocate 2 Huge Pages (4MB total)
    float *v_a = (float *)vmm_alloc_huge_page(1, VMM_WRITEABLE);
    float *v_b = (float *)vmm_alloc_huge_page(1, VMM_WRITEABLE);

    // 2. Mock some data (until your Streamer is done)
    for (uint32_t i = 0; i < 524288; i++)
    {
        v_a[i] = 1.0f;
        v_b[i] = 1.0f;
    }

    // 3. Run the Science
    float result = run_heavy_workload(v_a, v_b);

    // 4. Report
    PRINTS("Result of 2MB Dot Product: ");
    PRINTD((uint64_t)result); // Cast to int for your simple PRINTD
    PRINTLN;
}