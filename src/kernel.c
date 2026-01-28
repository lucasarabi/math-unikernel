#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "limine.h"

#include "libmu/lib_mu.c" 
#include "libmu/io_mu.c"
#include "libmu/pmm_mu.c"

#define PRINTS write_serial_str
#define PRINTH write_serial_hex
#define PRINTLN write_serial_str("\n");

__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(0);


__attribute__((used, section(".limine_requests_start")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end")))
static volatile LIMINE_REQUESTS_END_MARKER;

__attribute__((used, section(".limine_requests")))
static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0,
    .response = NULL
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0,
    .response = NULL   
};

void kernel_main(void) {
    
    serial_init(); 

    if(!LIMINE_BASE_REVISION_SUPPORTED) {
        hcf();
    }
    if(memmap_request.response == NULL) {
        PRINTS("ERROR: memmap request failed\n");
        hcf();
    }
    if(hhdm_request.response == NULL) {
        PRINTS("ERROR: hhdm request failed\n");
        hcf();
    }

    PRINTS("Limine handshake successful\n");

    PRINTS("Finding RAM ceiling\n");
    uint64_t highest_addr = get_highest_usable_addr(memmap_request.response);
    PRINTS("Highest address found: ");
    PRINTH(highest_addr);
    PRINTLN;

    PRINTS("Exiting...\n");
    hcf();
}
