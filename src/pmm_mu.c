#include <stdbool.h>
#include "headers/pmm_mu.h"
#include "headers/limine.h"
#include "headers/lib_mu.h"
#include "headers/io_mu.h"


#define PRINTS write_serial_str
#define PRINTD write_serial_dec
#define PRINTH write_serial_hex
#define PRINTLN write_serial_str("\n");
#define PRINTF(str, val) PRINTS(str); PRINTS(" "); PRINTD(val); PRINTLN;

#define PMM_UNINITIALIZED   "ERROR: Physical memory manager has not been initialized"
#define PMM_MISALIGNMENT    "ERROR: Attempted to free unaligned address. Pages but be aligned to 1kb (4096 bits)"


struct pmm_bitmap pmm;

static inline uint64_t phys_addr_to_index(uint64_t addr) {
    return addr / 4096;
}

static inline uint64_t index_to_phys_addr(uint64_t index) {
    return index * 4096;
}

static inline void claim_frame(uint8_t* bitmap, uint64_t frame_index) {
    uint64_t byte_index = frame_index / 8;
    uint64_t bit_index = frame_index % 8; 

    if((bitmap[byte_index] & (1 << bit_index)) == 0) {
        bitmap[byte_index] |= (1 << bit_index);
        pmm.free_frames--;
    }
}

static inline void free_frame(uint8_t* bitmap, uint64_t frame_index) {
    uint64_t byte_index = frame_index / 8;
    uint64_t bit_index = frame_index % 8; 
    if((bitmap[byte_index] & (1 << bit_index)) != 0) {
        bitmap[byte_index] &= ~(1 << bit_index);
        pmm.free_frames++;
    }
}

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

void pmm_init(struct limine_memmap_response* response, uint64_t hhdm_offset) {

    const uint64_t num_entries = response->entry_count;
    struct limine_memmap_entry** entries = response->entries;

    uint64_t ceiling_addr = get_highest_usable_addr(response); 
    uint64_t total_frames = (ceiling_addr + 4095) / 4096;
    uint64_t bitmap_size = (total_frames + 7) / 8;

    // Get start address -- earliest opening in usable physical memory
    uint64_t bitmap_physical_addr = 0;
    for(uint64_t i = 0; i < num_entries; i++) {
        struct limine_memmap_entry* current = entries[i];
        bool big_enough = current->length >= bitmap_size;

        if(current->type == LIMINE_MEMMAP_USABLE && big_enough) {
            bitmap_physical_addr = current->base;
            break;
        }
    }

    uint8_t* bitmap_virtual_ptr = (uint8_t*)(bitmap_physical_addr + hhdm_offset);

    memset(bitmap_virtual_ptr, 0xFF, bitmap_size);
    uint8_t* bitmap = bitmap_virtual_ptr; 

    pmm.bitmap = bitmap;
    pmm.bitmap_size = bitmap_size;
    pmm.total_frames = total_frames;
    pmm.free_frames = 0;

    for(uint64_t i = 0; i < num_entries; i++) {
        struct limine_memmap_entry* current = entries[i];
        if(current->type == LIMINE_MEMMAP_USABLE) {
            for(uint64_t addr = current->base; addr < (current->base + current->length); addr += 4096) {
                uint64_t frame_index = phys_addr_to_index(addr);
                free_frame(bitmap, frame_index); 
            }
        }
    }

    for(uint64_t addr = bitmap_physical_addr; addr < (bitmap_physical_addr + bitmap_size); addr += 4096) {
        uint64_t frame_index = phys_addr_to_index(addr);
        claim_frame(bitmap, frame_index);
    }
}

uint64_t pmm_alloc() {
    uint8_t* bitmap = pmm.bitmap;
    uint64_t bitmap_size = pmm.bitmap_size;
    uint64_t total_frames = pmm.total_frames;

    uint64_t byte_index = 0;
    while(byte_index < bitmap_size && bitmap[byte_index] == 0xff) {
        byte_index++;
    }

    if(byte_index >= bitmap_size) return 0;
    
    uint8_t bit_index = 0;
    while(bitmap[byte_index] & (1 << bit_index)) {
        bit_index++;
    }
    
    // DEBUG
    PRINTS("Byte index: "); PRINTD(byte_index);
    PRINTS(" Bit index: "); PRINTD(bit_index); 
    PRINTLN;

    uint64_t frame_index = (byte_index * 8) + bit_index; 

    if(frame_index >= total_frames) return 0;
    
    claim_frame(bitmap, frame_index);

    return index_to_phys_addr(frame_index);
}

void pmm_free(uint64_t phys_addr) {
    if((phys_addr & 0xfff) != 0) {
        PRINTS(PMM_MISALIGNMENT); PRINTLN;
        return;
    }

    uint8_t* bitmap = pmm.bitmap;

    uint64_t frame_index = phys_addr_to_index(phys_addr); 
    free_frame(bitmap, frame_index);
}
