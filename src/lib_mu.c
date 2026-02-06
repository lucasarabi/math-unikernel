#include "headers/lib_mu.h"

void* memcpy(void* restrict dest, const void* restrict src, size_t n) {
    uint8_t* restrict pdest = (uint8_t* restrict)dest;
    const uint8_t* restrict psrc = (const uint8_t* restrict) src;    

    for(size_t i = 0; i < n; i++) {
        pdest[i] = psrc[i];
    }    

    return dest;
}

/*
 * s is start addr
 * c is memset value
 * is number of bytes
 */
void* memset(void* s, uint8_t c, size_t n) {
    uint8_t* p = (uint8_t*)s;

    for(size_t i = 0; i < n; i++) {
        p[i] = (uint8_t)c;
    }
        
    return s;
}

void* memmove(void* dest, const void* src, size_t n) {
    uint8_t* pdest = (uint8_t*)dest;
    const uint8_t* psrc = (const uint8_t*)src;
    
    if(pdest == psrc) return dest;
        
    if(src > dest) {
        for(size_t i = 0; i < n; i++) {
            pdest[i] = psrc[i];
        }
    } else if(src < dest) {
        for(size_t i = n; i > 0; i--) {
            pdest[i-1] = psrc[i-1];
        }
    }

    return dest;
}

int memcmp(const void* src1, const void* src2, size_t n) {
    const uint8_t* p1 = (const uint8_t*)src1;
    const uint8_t* p2 = (const uint8_t*)src2;

    if(p1 == p2) return 0; 

    for(size_t i = 0; i < n; i++) {
        if(p1[i] != p2[i])
            return p1[i] < p2[i] ? -1 : 1;
    }

    return 0;
}

void hcf(void) {
    for(;;) {
        asm("hlt");
    }
}

