#ifndef MATHLIB_MU_H
#define MATHLIB_MU_H

#include <stdint.h>

float dot_product(float* a, float* b, uint64_t count);
void  matrix_multiply(float* a, float* b, float* out, uint64_t n);

#endif