#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "headers/limine.h"
#include "headers/hhdm_offset.h"
#include "headers/lib.h"
#include "headers/io.h"
#include "headers/pmm.h"
#include "headers/vmm.h"
#include "headers/idt.h"
#include "headers/gdt.h"

#define PRINTS write_serial_str
#define PRINTH write_serial_hex
#define PRINTD write_serial_dec
#define PRINTB(val) write_serial_bin(val, 8)
#define PRINTLN write_serial_str("\n");
#define PRINTF(str, val) PRINTS(str); PRINTS("\t"); PRINTD(val); PRINTS("\t");

#define MEMMAP_REQUEST_FAILURE      "ERROR: memmap request failed.\n"
#define HHDM_REQUEST_FAILURE        "ERROR: hhdm request failed.\n"
#define KERNEL_ADDR_REQUEST_FAILURE "ERROR: kernel address request failed.\n"
#define PMM_INITIALIZED             "PMM has been initialized.\n"
#define VMM_INITIALIZED             "VMM has been initialized and loaded.\n"
#define IDT_INITIALIZED             "IDT has been initialized and loaded.\n"
#define GDT_INITIALIZED             "GDT has been initialized and loaded.\n"

#define LIMINE_HANDSHAKE_SUCCESS    "Limine handshake successful.\n"
#define KERNEL_FINISH   "Finished kernel execution. Exiting.\n"

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

__attribute__((used, section(".limine_requests")))
static volatile struct limine_kernel_address_request kernel_addr_request = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 0,
    .response = NULL
};

uint64_t hhdm_offset;

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
    if(kernel_addr_request.response == NULL) {
        PRINTS(KERNEL_ADDR_REQUEST_FAILURE);
        hcf();
    }

    PRINTS(LIMINE_HANDSHAKE_SUCCESS); 

    hhdm_offset = hhdm_request.response->offset;

    pmm_init(memmap_request.response); 
    PRINTS(PMM_INITIALIZED);

    vmm_init(kernel_addr_request.response, memmap_request.response);
    PRINTS(VMM_INITIALIZED); 

    idt_init(); 
    PRINTS(IDT_INITIALIZED); 

    gdt_init();
    PRINTS(GDT_INITIALIZED); 

    PRINTS(KERNEL_FINISH);
    hcf();
}
