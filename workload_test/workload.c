#pragma GCC target("avx,fma")

#include <stdint.h>
#include <immintrin.h>
#include "kernel_api_import.h"

static kernel_api_t* api = (kernel_api_t*)KERNEL_API_ADDRESS;

MAIN void run() {
    api->prints("Hello world\n");
}