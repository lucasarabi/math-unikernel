#include "headers/kernel_api.h"
#include "headers/limine.h"
#include "headers/hhdm_offset.h"
#include "headers/lib.h"
#include "headers/io.h"
#include "headers/pmm.h"
#include "headers/vmm.h"
#include "headers/idt.h"
#include "headers/gdt.h"
#include "headers/loader.h"
#include "headers/states.h"
#include "headers/display.h"
#include "headers/pci.h"
#include "headers/pic.h"
#include "headers/network.h"
#include "headers/mathlib.h"
#include "headers/kernel_logs.h"

#define LIMINE_HANDSHAKE_SUCCESS        (1<<0)

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

void delay_ms(uint32_t ms) {
    for (uint32_t i = 0; i < ms; i++)
        for (uint32_t j = 0; j < 500; j++)
            outb(0x80, 0);
}

void kernel_main(void) {

    uint16_t init_status = 0;

    if(LIMINE_BASE_REVISION_SUPPORTED
        && memmap_request.response != NULL
        && hhdm_request.response != NULL
        && kernel_addr_request.response != NULL
        && framebuffer_request.response != NULL
        && framebuffer_request.response->framebuffer_count >= 1)
    {
        init_status |= LIMINE_HANDSHAKE_SUCCESS;
    }

    struct limine_framebuffer* fb = framebuffer_request.response->framebuffers[0];
    hhdm_offset = hhdm_request.response->offset;

    pic_remap(32, 40);

    init_status |= gdt_init();
    init_status |= idt_init();
    init_status |= pmm_init(memmap_request.response);
    init_status |= vmm_init(kernel_addr_request.response, memmap_request.response);
    init_status |= enable_simd();
    init_status |= display_init((uint32_t *)fb->address, fb->pitch, fb->width, fb->height);
    init_status |= serial_init(115200);
    init_status |= pci_scan_bus();
    init_status |= timer_calibrate();
    PRINTLN;

    if(init_status & LIMINE_HANDSHAKE_SUCCESS)  PRINTS(LIMINE_SUCCESS_LOG);         else { PRINTS(LIMINE_FAILURE_LOG);          hcf(); }
    if(init_status & GDT_INIT_SUCCESS)          PRINTS(GDT_INITIALIZED);            else { PRINTS(GDT_FAILURE);                 hcf(); }
    if(init_status & IDT_INIT_SUCCESS)          PRINTS(IDT_INITIALIZED);            else { PRINTS(IDT_FAILURE);                 hcf(); }
    if(init_status & PMM_INIT_SUCCESS)          PRINTS(PMM_INITIALIZED);            else { PRINTS(PMM_FAILURE);                 hcf(); }
    if(init_status & VMM_INIT_SUCCESS)          PRINTS(VMM_INITIALIZED);            else { PRINTS(VMM_FAILURE);                 hcf(); }
    if(init_status & DISPLAY_INIT_SUCCESS)      PRINTS(DISPLAY_INITIALIZED);        else {                                      hcf(); }
    if(init_status & SERIAL_INIT_SUCCESS)       PRINTS(SERIAL_DRIVER_INITIALIZED);  else { PRINTS(SERIAL_DRIVER_FAILURE);       hcf(); }
    if(init_status & PCI_INIT_SUCCESS)          PRINTS(NETWORK_CONTROLLER_FOUND);   else { PRINTS(NETWORK_CONTROLLER_MISSING);  hcf(); }
    if(init_status & TIMER_INIT_SUCCESS)        PRINTS(TIMER_CALIBRATED);           else { PRINTS(TIMER_CALIBRATION_FAILURE);   hcf(); }
    PRINTLN;

    uint64_t start = rdtscp();
    uint64_t timeout_cycles = ms_to_cycles(5000); // 5 seconds

    while ((rdtscp() - start) < timeout_cycles)
        ;

    fb_clear(); 

    __asm__ volatile("sti"); // Enable maskable hardware interrupts after completing boot sequence
    
    vmm_map_range(KERNEL_API_ADDRESS, KERNEL_API_ADDRESS, 4096, VMM_PRESENT | VMM_WRITEABLE);
    kernel_api_t *api = (kernel_api_t *)KERNEL_API_ADDRESS;
    api->alloc_huge_page = vmm_alloc_huge_page;
    api->dot_product = dot_product;
    api->matrix_multiply = matrix_multiply;
    api->spmv_csr = spmv_csr;
    api->init_vector_deterministic = init_vector_deterministic;
    api->init_matrix_deterministic = init_matrix_deterministic;
    api->generate_banded_matrix = generate_banded_matrix;
    api->rdtscp = rdtscp;
    api->cycles_to_ms = cycles_to_ms;
    api->printd = fb_print_dec;
    api->prints = fb_print;
    api->set_output = NULL; // implement if needed
    api->output_buffer = NULL;
    api->output_size = 0;

    enum states state = POLLING;
    bool running = true;
    uint8_t* payload_mem;

    do {
        switch(state) {
            case POLLING:
                reset_header();
                api->output_buffer = NULL;
                api->output_size = 0;

                PRINTS(STATE_POLLING);

                unlock();

                uint64_t payload_byte_count = poll_payload_size();
                PRINTF("Payload byte size:", payload_byte_count); PRINTLN;

                uint64_t num_pages = payload_byte_count / (2 * MB);
                if (payload_byte_count % (2 * MB) != 0)
                    num_pages++;

                payload_mem = vmm_alloc_huge_page(num_pages, VMM_PRESENT | VMM_WRITEABLE);

                PRINTS("Downloading workload.\n");
                poll_payload(payload_mem, payload_byte_count);
                PRINTS("Workload downloaded.\n");

                PRINTLN;
                state = EXECUTING;
                break;

            case EXECUTING:
                PRINTS(STATE_EXECUTING);

                void (*workload)() = (void (*)())payload_mem;
                workload();

                PRINTLN;
                state = EXTRACTING;
                break;

            case EXTRACTING:
                PRINTS(STATE_EXTRACTING);

                if(api->output_buffer && api->output_size > 0) {
                    // network_send_frame((uint8_t*)api->output_buffer, api->output_size);
                }
                else {
                    // PRINTS("Output buffer empty. Continuing. \n");
                }

                PRINTLN;
                state = POLLING;    
                break;
        }
    } while(running);

    PRINTS(KERNEL_FINISH);
    hcf();
}