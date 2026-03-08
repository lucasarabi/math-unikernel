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
#include "headers/pci.h"

#define LIMINE_HANDSHAKE_SUCCESS        (1<<0)

#define LIMINE_SUCCESS_LOG              "READY: Limine handshake successful.\n"
#define GDT_INITIALIZED                 "READY: GDT has been initialized and loaded.\n"
#define IDT_INITIALIZED                 "READY: IDT has been initialized and loaded.\n"
#define PMM_INITIALIZED                 "READY: PMM has been initialized.\n"
#define VMM_INITIALIZED                 "READY: VMM has been initialized and loaded.\n"
#define SIMD_ENABLED                    "READY: AVX/SSE enabled.\n"
#define DISPLAY_INITIALIZED             "READY: Display has been initialized.\n"
#define SERIAL_DRIVER_INITIALIZED       "READY: Serial drivers have been initialized.\n"
#define NETWORK_CONTROLLER_FOUND        "READY: Network Contoller found on PCI bus.\n"

#define LIMINE_FAILURE_LOG              "ERROR: Limine handshake failed.\n"
#define GDT_FAILURE                     "ERROR: GDT initialization failed.\n"
#define IDT_FAILURE                     "ERROR: IDT initialization failed.\n"
#define PMM_FAILURE                     "ERROR: PMM initialization failed.\n"
#define VMM_FAILURE                     "ERROR: VMM initialization failed.\n"
#define SIMD_FAILURE                    "ERROR: AVX/SSE not supported by CPU.\n"
#define SERIAL_DRIVER_FAILURE           "ERROR: Serial driver initialization failed.\n"
#define NETWORK_CONTROLLER_MISSING      "ERROR: Network Controller not found on PCI bus.\n"

#define STATE_POLLING                   "STATE: Polling\n"
#define STATE_EXECUTING                 "STATE: Executing\n"
#define STATE_EXTRACTING                "STATE: Extracting\n"

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

    uint16_t init_status = 0;
    
    if(LIMINE_BASE_REVISION_SUPPORTED 
        || memmap_request.response != NULL
        || hhdm_request.response != NULL
        || kernel_addr_request.response != NULL
        || framebuffer_request.response != NULL 
        || framebuffer_request.response->framebuffer_count >= 1) 
    {
        init_status |= LIMINE_HANDSHAKE_SUCCESS;
    }
    struct limine_framebuffer* fb = framebuffer_request.response->framebuffers[0];
    hhdm_offset = hhdm_request.response->offset;

    init_status |= gdt_init();
    init_status |= idt_init(); 
    init_status |= pmm_init(memmap_request.response); 
    init_status |= vmm_init(kernel_addr_request.response, memmap_request.response);
    init_status |= enable_simd();
    init_status |= display_init((uint32_t *)fb->address, fb->pitch, fb->width, fb->height);
    init_status |= serial_init(115200);
    init_status |= pci_scan_bus();

    PRINTLN;
    if(init_status & LIMINE_HANDSHAKE_SUCCESS)  PRINTS(LIMINE_SUCCESS_LOG);         else { PRINTS(LIMINE_FAILURE_LOG);          hcf();}
    if(init_status & GDT_INIT_SUCCESS)          PRINTS(GDT_INITIALIZED);            else { PRINTS(GDT_FAILURE);                 hcf();}
    if(init_status & IDT_INIT_SUCCESS)          PRINTS(IDT_INITIALIZED);            else { PRINTS(IDT_FAILURE);                 hcf();}
    if(init_status & PMM_INIT_SUCCESS)          PRINTS(PMM_INITIALIZED);            else { PRINTS(PMM_FAILURE);                 hcf();}
    if(init_status & VMM_INIT_SUCCESS)          PRINTS(VMM_INITIALIZED);            else { PRINTS(VMM_FAILURE);                 hcf();}
    if(init_status & DISPLAY_INIT_SUCCESS)      PRINTS(DISPLAY_INITIALIZED);        else { /* You'll know lol*/                 hcf();}
    if(init_status & SERIAL_INIT_SUCCESS)       PRINTS(SERIAL_DRIVER_INITIALIZED);  else { PRINTS(SERIAL_DRIVER_FAILURE);       hcf();}
    if(init_status & PCI_INIT_SUCCESS)          PRINTS(NETWORK_CONTROLLER_FOUND);   else { PRINTS(NETWORK_CONTROLLER_MISSING);  hcf();}
    PRINTLN;

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

                PRINTLN;
                state = EXECUTING;
                break;

            case EXECUTING:
                PRINTS(STATE_EXECUTING);

                // run();

                PRINTLN;
                state = EXTRACTING;
                break;

            case EXTRACTING:
                PRINTS(STATE_EXTRACTING);

                PRINTLN;            // temp    
                state = POLLING;    // temp
                // running = false;
                break;
        }
    } while(running);

    PRINTS(KERNEL_FINISH);
    hcf();
}
