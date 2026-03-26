#pragma GCC target("avx,fma")

#include <stdint.h>
#include <immintrin.h>
#include "kernel_api_import.h"

MAIN void run() {
    kernel_api_t* api = (kernel_api_t*)KERNEL_API_ADDRESS;

    volatile uint8_t a = 1;
    volatile uint8_t b = 2;

    volatile uint8_t c = a + b;

    api->prints("a + b = ");
    api->printd(c);
    api->prints("\n");
}