#ifndef MATHLIB_MU_H
#define MATHLIB_MU_H
#include <stdint.h>

// Compute bound
float dot_product(float *a, float *b, uint64_t count);
void matrix_multiply(float *a, float *b, float *out, uint64_t n);

// Memory bound
void spmv_csr(float *values, uint32_t *col_idx, uint32_t *row_ptr,
              float *x, float *y, uint64_t num_rows);

// Init helpers
void init_vector_deterministic(float *vec, uint64_t count);
void init_matrix_deterministic(float *mat, uint64_t rows, uint64_t cols);
uint64_t generate_banded_matrix(float *values, uint32_t *col_idx, uint32_t *row_ptr,
                                uint64_t num_rows, uint64_t band_width);

#endif
