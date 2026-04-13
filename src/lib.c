#include "headers/lib.h"
#include "headers/display.h"

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

/*
 * TIMER FUNCTIONS
 */

#define PIT_FREQ_HZ      1193182
#define PIT_CHANNEL0     0x40
#define PIT_COMMAND      0x43

static uint64_t tsc_freq_hz = 0;

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static uint16_t pit_read_count(void) {
    outb(PIT_COMMAND, 0x00);
    uint8_t lo = inb(PIT_CHANNEL0);
    uint8_t hi = inb(PIT_CHANNEL0);
    return ((uint16_t)hi << 8) | lo;
}

uint64_t rdtscp(void) {
    uint32_t lo, hi, aux;
    asm volatile ("rdtscp" : "=a"(lo), "=d"(hi), "=c"(aux));
    return ((uint64_t)hi << 32) | lo;
}

uint16_t timer_calibrate(void) {
    asm volatile ("cli");
    
    outb(PIT_COMMAND, 0x30);
    outb(PIT_CHANNEL0, 0xFF);
    outb(PIT_CHANNEL0, 0xFF);
    
    for (volatile int i = 0; i < 100; i++);
    
    uint16_t pit_start = pit_read_count();
    uint64_t tsc_start = rdtscp();
    
    uint16_t pit_now;
    do {
        pit_now = pit_read_count();
    } while ((pit_start - pit_now) < 50000 && pit_now <= pit_start);
    
    uint16_t pit_elapsed = (pit_now <= pit_start) 
        ? (pit_start - pit_now) 
        : (pit_start + (0xFFFF - pit_now) + 1);
    
    uint64_t tsc_elapsed = rdtscp() - tsc_start;

    if (pit_elapsed == 0) {
        asm volatile ("sti");
        return 0;
    }

    tsc_freq_hz = (tsc_elapsed * PIT_FREQ_HZ) / pit_elapsed;
    PRINTS("TSC freq: "); PRINTD(tsc_freq_hz); PRINTLN;

    if (tsc_freq_hz == 0) {
        asm volatile ("sti");
        return 0;
    }

    asm volatile ("sti");

    return TIMER_INIT_SUCCESS;
}

uint64_t cycles_to_ms(uint64_t cycles) {
    if (tsc_freq_hz == 0) return 0;
    return (cycles * 1000) / tsc_freq_hz;
}

uint64_t ms_to_cycles(uint64_t ms) {
    return (ms * tsc_freq_hz) / 1000;
}