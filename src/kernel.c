#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "limine.h"

#include "libmu/libmu.c" 
#include "libmu/muio.c"

__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(0);
//static volatile uint64_t limine_base_revision[] = LIMINE_BASE_REVISION(4);

// Skipping framebuffer request -- we do not need graphics

__attribute__((used, section(".limine_requests_start")))
static volatile LIMINE_REQUESTS_START_MARKER;
//static volatile uint64_t limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end")))
static volatile LIMINE_REQUESTS_END_MARKER;
//static volatile uint64_t limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;


void kernel_main(void) {
    
    if(!LIMINE_BASE_REVISION_SUPPORTED) {
        hcf();
    }

    // Skipping framebuffer response check 

    // Send data to serial port
    //outb(0x3f8, 'S'); 
    
    // Test print
    print_literal("hello world!");
    print_literal("I've booted!");

    hcf();
}
