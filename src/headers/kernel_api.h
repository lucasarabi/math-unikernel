#ifndef KERNEL_API_MU_H
#define KERNEL_API_MU_H
#include <stdint.h>
#define KERNEL_API_ADDRESS 0x10000000

typedef struct {
    // Memory
    void*    (*alloc_huge_page)(uint64_t n, uint64_t flags);
    
    // Math - compute bound
    float    (*dot_product)(float* a, float* b, uint64_t count);
    void     (*matrix_multiply)(float* a, float* b, float* out, uint64_t n);
    
    // Math - memory bound
    void     (*spmv_csr)(float* values, uint32_t* col_idx, uint32_t* row_ptr,
                         float* x, float* y, uint64_t num_rows);
    
    // Math - init helpers
    void     (*init_vector_deterministic)(float* vec, uint64_t count);
    void     (*init_matrix_deterministic)(float* mat, uint64_t rows, uint64_t cols);
    uint64_t (*generate_banded_matrix)(float* values, uint32_t* col_idx, uint32_t* row_ptr,
                                       uint64_t num_rows, uint64_t band_width);
    
    // Timer
    uint64_t (*rdtscp)(void);
    uint64_t (*cycles_to_ms)(uint64_t cycles);
    
    // Output
    void     (*printd)(uint64_t);
    void     (*prints)(const char*);
    void     (*set_output)(void* buffer, uint64_t size);
    void*    output_buffer;
    uint64_t output_size;
} kernel_api_t;

#endif