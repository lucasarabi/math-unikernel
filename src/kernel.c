#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "limine.h"

#include "libmu/libmu.c"

__attribute__((used, section(".limine_requests")))
static volatile uint64_t limine_base_revision[] = LIMINE_BASE_REVISION(0);

// Skipping framebuffer request -- we do not need graphics

__attribute__((used, section(".limine_requests")))
static volatile uint64_t limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests")))
static volatile uint64_t limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;

void kernel_main(void) {
    
    if(LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision == false)) {
        hcf();
    }

    // Skipping framebuffer response check 

    

}
