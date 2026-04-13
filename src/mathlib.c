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

/*
 * Sparse Matrix-Vector Multiply (CSR format)
 * 
 * Computes y = A * x where A is sparse in CSR format.
 * 
 * Parameters:
 *   values   - Non-zero values array (length = row_ptr[num_rows])
 *   col_idx  - Column indices for each non-zero (same length as values)
 *   row_ptr  - Row pointers: row i spans values[row_ptr[i]] to values[row_ptr[i+1]-1]
 *              Length = num_rows + 1
 *   x        - Input vector (length >= max(col_idx))
 *   y        - Output vector (length = num_rows), must be zeroed by caller or will accumulate
 *   num_rows - Number of rows in matrix
 *
 * Memory alignment:
 *   - values must be 32-byte aligned
 *   - x must be 32-byte aligned  
 *   - y must be 32-byte aligned
 *   - col_idx and row_ptr have no alignment requirement
 *
 * Notes:
 *   - Uses AVX for rows with 8+ consecutive elements
 *   - Falls back to scalar for short rows / tail elements
 *   - For best performance, pad values array to multiple of 8 per row
 */
void spmv_csr(float* values, uint32_t* col_idx, uint32_t* row_ptr,
              float* x, float* y, uint64_t num_rows) {
    
    for (uint64_t row = 0; row < num_rows; row++) {
        uint32_t start = row_ptr[row];
        uint32_t end = row_ptr[row + 1];
        
        __m256 sum_vec = _mm256_setzero_ps();
        float sum_scalar = 0.0f;
        
        uint32_t i = start;
        
        // Process 8 elements at a time with AVX
        // Note: We can't use aligned loads on x because col_idx gives arbitrary indices
        // But values array is contiguous and aligned
        for (; i + 8 <= end; i += 8) {
            // Load 8 values (aligned, contiguous in values array)
            __m256 v = _mm256_load_ps(&values[i]);
            
            // Gather 8 x values using column indices
            // AVX2 gather: slower than manual but cleaner
            // Manual approach for better compatibility:
            __m256 x_vec = _mm256_set_ps(
                x[col_idx[i + 7]],
                x[col_idx[i + 6]],
                x[col_idx[i + 5]],
                x[col_idx[i + 4]],
                x[col_idx[i + 3]],
                x[col_idx[i + 2]],
                x[col_idx[i + 1]],
                x[col_idx[i + 0]]
            );
            
            sum_vec = _mm256_fmadd_ps(v, x_vec, sum_vec);
        }
        
        // Horizontal sum of AVX register
        float temp[8] __attribute__((aligned(32)));
        _mm256_store_ps(temp, sum_vec);
        for (int k = 0; k < 8; k++) {
            sum_scalar += temp[k];
        }
        
        // Handle remaining elements (tail)
        for (; i < end; i++) {
            sum_scalar += values[i] * x[col_idx[i]];
        }
        
        y[row] = sum_scalar;
    }
}

/*
 * Generate a deterministic banded sparse matrix in CSR format.
 * 
 * Creates a matrix with a fixed sparsity pattern:
 *   - Main diagonal
 *   - band_width diagonals above and below main diagonal
 * 
 * This gives (2 * band_width + 1) non-zeros per row (except near edges).
 * Total nnz ≈ num_rows * (2 * band_width + 1)
 *
 * Parameters:
 *   values   - Output: non-zero values (must be pre-allocated, 32-byte aligned)
 *   col_idx  - Output: column indices (must be pre-allocated)
 *   row_ptr  - Output: row pointers (must be pre-allocated, length = num_rows + 1)
 *   num_rows - Matrix dimension (square matrix: num_rows x num_rows)
 *   band_width - Number of diagonals above/below main diagonal
 *
 * Returns: total number of non-zeros written
 *
 * Value pattern: values are deterministic based on position.
 *   value = 1.0 / (1 + |row - col|)
 *   Main diagonal = 1.0, off-diagonals decay
 */
uint64_t generate_banded_matrix(float* values, uint32_t* col_idx, uint32_t* row_ptr,
                                 uint64_t num_rows, uint64_t band_width) {
    uint64_t nnz = 0;
    
    for (uint64_t row = 0; row < num_rows; row++) {
        row_ptr[row] = (uint32_t)nnz;
        
        // Determine column range for this row
        int64_t col_start = (int64_t)row - (int64_t)band_width;
        int64_t col_end = (int64_t)row + (int64_t)band_width;
        
        if (col_start < 0) col_start = 0;
        if (col_end >= (int64_t)num_rows) col_end = (int64_t)num_rows - 1;
        
        for (int64_t col = col_start; col <= col_end; col++) {
            col_idx[nnz] = (uint32_t)col;
            
            // Deterministic value based on distance from diagonal
            int64_t dist = row > (uint64_t)col ? (int64_t)row - col : col - (int64_t)row;
            values[nnz] = 1.0f / (1.0f + (float)dist);
            
            nnz++;
        }
    }
    
    row_ptr[num_rows] = (uint32_t)nnz;
    return nnz;
}

/*
 * Initialize a vector with deterministic values.
 * Pattern: vec[i] = sin(i * 0.01) using Taylor series approximation
 * (No libm dependency)
 */
void init_vector_deterministic(float* vec, uint64_t count) {
    for (uint64_t i = 0; i < count; i++) {
        // Simple deterministic pattern without trig
        // Produces values in predictable range
        float t = (float)(i % 1000) * 0.001f;  // 0.0 to 0.999
        vec[i] = t * (1.0f - t) * 4.0f;        // Parabola, peak at 0.5, range [0, 1]
    }
}

/*
 * Initialize a matrix with deterministic values for benchmarking.
 * Pattern creates non-trivial but reproducible data.
 */
void init_matrix_deterministic(float* mat, uint64_t rows, uint64_t cols) {
    for (uint64_t i = 0; i < rows; i++) {
        for (uint64_t j = 0; j < cols; j++) {
            // Deterministic pattern
            float val = (float)((i * 7 + j * 13) % 1000) * 0.001f;
            mat[i * cols + j] = val;
        }
    }
}