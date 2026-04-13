#ifndef LIB_MU_H
#define LIB_MU_H

#include <stddef.h>
#include <stdint.h>

#define SIMD_INIT_SUCCESS   (1<<5)
#define TIMER_INIT_SUCCESS   (1<<9)

// Memory manipulation API
void* memset(void* s, uint8_t c, size_t n);
void* memcpy(void* restrict dest, const void* restrict src, size_t n);
uint8_t enable_simd();
void hcf();

// Timer API
uint64_t rdtscp(void);              // Get cycle count
uint16_t timer_calibrate(void);     // Call once at boot
uint64_t cycles_to_ms(uint64_t);    
uint64_t ms_to_cycles(uint64_t ms); 

#endif
