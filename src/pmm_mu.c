#include "headers/pmm_mu.h"
#include "headers/limine.h"
#include "headers/lib_mu.h"
#include "headers/io_mu.h"


#define PRINTS write_serial_str
#define PRINTD write_serial_dec
#define PRINTH write_serial_hex
#define PRINTLN write_serial_str("\n");


#define PMM_UNINITIALIZED "ERROR: Physical memory manager has not been initialized"


struct pmm_bitmap pmm;

static uint64_t get_highest_usable_addr(struct limine_memmap_response* response) {

    if(response->entries == NULL) return 0;

    const uint64_t num_entries = response->entry_count;
    struct limine_memmap_entry** entries = response->entries;
    uint64_t largest_addr = 0;    

    for(uint64_t i = 0; i < num_entries; i++) {
        struct limine_memmap_entry* current = entries[i];

        if(current == NULL) {
            continue;
        }

        uint64_t ceiling = current->base + current->length;

        if(current->type == LIMINE_MEMMAP_USABLE && ceiling > largest_addr) 
            largest_addr = ceiling;
    }

    return largest_addr;
}


// addr refers to PHYSICAL MEMORY address
static inline uint64_t addr_to_index(uint64_t addr) {
    return addr / 4096;
}

static inline uint64_t index_to_addr(uint64_t index) {
    return index * 4096;
}


static inline void claim_frame(uint8_t* bitmap, uint64_t frame_index) {
    uint64_t byte_index = frame_index / 8;
    uint64_t bit_index = frame_index % 8; 

    bitmap[byte_index] |= (1 << bit_index);

}

static inline void free_frame(uint8_t* bitmap, uint64_t frame_index) {
    uint64_t byte_index = frame_index / 8;
    uint64_t bit_index = frame_index % 8; 

    bitmap[byte_index] &= ~(1 << bit_index);
}


void update_pmm_frame_count() {
    if(pmm.bitmap == NULL) {
        PRINTS(PMM_UNINITIALIZED); PRINTLN;
        hcf();
    }

    uint8_t* bitmap = pmm.bitmap;
    uint64_t total_frames = pmm.total_frames;

    uint64_t num_free_frames = 0;
    uint64_t num_claimed_frames = 0;

    for (uint64_t frame = 0; frame < total_frames; frame++) {
        uint64_t byte_index = frame / 8;
        uint64_t bit_index = frame % 8;

        // Below is the logic to determine frame status!
        // TODO -- abstract this logic to function. Contruct pmm API using these as well as free_frame and claim_frame
        if ((bitmap[byte_index] & (1 << bit_index)) == 0) num_free_frames++;
        else num_claimed_frames++;
    }
    
    pmm.total_free_frames = num_free_frames;
    pmm.total_claimed_frames = num_claimed_frames;

 }


void pmm_init(struct limine_memmap_response* response) {

    const uint64_t num_entries = response->entry_count;
    struct limine_memmap_entry** entries = response->entries;

    uint64_t ceiling_addr = get_highest_usable_addr(response); 
    uint64_t total_frames = (ceiling_addr + 4095) / 4096;
    pmm.total_frames = total_frames;
    uint64_t bitmap_size = (total_frames + 7) / 8;

    // Get start address -- earliest opening in usable physical memory
    uint8_t* bitmap_start = NULL;
    for(uint64_t i = 0; i < num_entries; i++) {
        struct limine_memmap_entry* current = entries[i];
        bool big_enough = current->length >= bitmap_size;

        if(current->type == LIMINE_MEMMAP_USABLE && big_enough) {
            bitmap_start = (uint8_t*)current->base;
            break;
        }
    }

    memset(bitmap_start, 0xFF, bitmap_size);
    uint8_t* bitmap = bitmap_start; 

    pmm.bitmap = bitmap;
    pmm.bitmap_size = bitmap_size;

    // Free up USABLE memory
    for(uint64_t i = 0; i < num_entries; i++) {
        struct limine_memmap_entry* current = entries[i];
        if(current->type == LIMINE_MEMMAP_USABLE) {
            for(uint64_t addr = current->base; addr < (current->base + current->length); addr += 4096) {
                uint64_t frame_index = addr_to_index(addr);
                free_frame(bitmap, frame_index); 
            }
        }
    }

    // Reclaim memory address storing pmm bitmap
    for(uint64_t addr = (uint64_t)bitmap; addr < ((uint64_t)bitmap + bitmap_size); addr += 4096) {
        uint64_t frame_index = addr_to_index(addr);
        claim_frame(bitmap, frame_index);
    }

    update_pmm_frame_count();

}


