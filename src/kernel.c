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
#include "headers/workload.h"
#include "headers/loader.h"
#include "headers/states.h"
#include "headers/display.h"

#define MEMMAP_REQUEST_FAILURE          "ERROR: memmap request failed.\n"
#define HHDM_REQUEST_FAILURE            "ERROR: hhdm request failed.\n"
#define KERNEL_ADDR_REQUEST_FAILURE     "ERROR: kernel address request failed.\n"
#define UNSUPPORTED_REVISION_FAILURE    "ERROR: kernel address request failed.\n"
#define FRAMEBUFFER_REQUEST_FAILURE     "ERROR: framebuffer request failed.\n"

#define PMM_INITIALIZED                 "READY: PMM has been initialized.\n"
#define VMM_INITIALIZED                 "READY: VMM has been initialized and loaded.\n"
#define IDT_INITIALIZED                 "READY: IDT has been initialized and loaded.\n"
#define GDT_INITIALIZED                 "READY: GDT has been initialized and loaded.\n"
#define SIMD_ENABLED                    "READY: AVX/SSE enabled.\n"
#define DISPLAY_INITIALIZED             "READY: Display initialized.\n"

#define STATE_POLLING                   "STATE: Polling\n"
#define STATE_EXECUTING                 "STATE: Executing\n"
#define STATE_EXTRACTING                "STATE: Extracting\n"

#define LIMINE_HANDSHAKE_SUCCESS        "Limine handshake successful.\n"
#define KERNEL_FINISH                   "Finished kernel execution. Exiting.\n"

#define MB (1ULL << 20)

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

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

uint64_t hhdm_offset;

void kernel_main(void) {
    
    serial_init(115200); 

    if(!LIMINE_BASE_REVISION_SUPPORTED) {
        PRINTS(UNSUPPORTED_REVISION_FAILURE);
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

       if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        PRINTS(FRAMEBUFFER_REQUEST_FAILURE);
        hcf();
    } 

    PRINTS(LIMINE_HANDSHAKE_SUCCESS); 

    hhdm_offset = hhdm_request.response->offset;
    
    gdt_init();

    idt_init(); 

    pmm_init(memmap_request.response); 

    vmm_init(kernel_addr_request.response, memmap_request.response);

    enable_simd();

    struct limine_framebuffer* fb = framebuffer_request.response->framebuffers[0];
    display_init((uint32_t *)fb->address, fb->pitch, fb->width, fb->height);

    enum states state = POLLING; 
    bool running = true;

    do {
        switch(state) {
            case POLLING:
                PRINTS(STATE_POLLING);

                PRINTTAB; PRINTS("Waiting for magic number."); PRINTLN;
                unlock();
                PRINTTAB; PRINTS("Unlocked."); PRINTLN;

                PRINTTAB; PRINTS("Awaiting payload size."); PRINTLN;
                uint64_t payload_byte_count = poll_payload_size();
                PRINTTAB; PRINTF("Payload byte size:", payload_byte_count); PRINTLN;

                uint64_t num_pages = payload_byte_count / (2*MB);
                if(payload_byte_count % (2*MB) != 0)
                    num_pages++;

                uint8_t* payload_mem = vmm_alloc_huge_page(num_pages, VMM_WRITEABLE);

                PRINTTAB; PRINTS("Downloading workload."); PRINTLN;
                poll_payload(payload_mem, payload_byte_count);
                PRINTTAB; PRINTS("Workload downloaded."); PRINTLN;

                state = EXECUTING;
                break;

            case EXECUTING:
                PRINTS(STATE_EXECUTING);

                // run();

                state = EXTRACTING;
                break;

            case EXTRACTING:
                PRINTS(STATE_EXTRACTING);

                state = POLLING; // temp
                // running = false;
                break;
        }
    } while(running);

    PRINTS(KERNEL_FINISH);
    hcf();
}
