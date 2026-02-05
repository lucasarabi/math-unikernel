#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "headers/limine.h"
#include "headers/lib_mu.h"
#include "headers/io_mu.h"
#include "headers/pmm_mu.h"

#define PRINTS write_serial_str
#define PRINTH write_serial_hex
#define PRINTD write_serial_dec
#define PRINTLN write_serial_str("\n");
#define PRINTF(str, val) PRINTS(str); PRINTS(" "); PRINTD(val); PRINTLN;

#define MEMMAP_REQUEST_FAILURE "ERROR: memmap request failed.\n"
#define HHDM_REQUEST_FAILURE "ERROR: hhdm request failed.\n"

#define LIMINE_HANDSHAKE_SUCCESS "Limine handshake successful.\n"
#define KERNEL_FINISH "Finished kernel execution. Exiting.\n"

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
        PRINTS(MEMMAP_REQUEST_FAILURE);
        hcf();
    }
    if(hhdm_request.response == NULL) {
        PRINTS(HHDM_REQUEST_FAILURE);
        hcf();
    }

    PRINTS(LIMINE_HANDSHAKE_SUCCESS); 

    pmm_init(memmap_request.response, hhdm_request.response->offset); 
    PRINTF("PMM total frames:", pmm.total_frames);
    PRINTF("PMM free frames:", pmm.free_frames);
    PRINTF("PMM used frames:", pmm.total_frames - pmm.free_frames);

    PRINTS(KERNEL_FINISH);
    hcf();
}
