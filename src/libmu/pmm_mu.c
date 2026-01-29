// Headers included in kernel.c

#define PRINTS write_serial_str
#define PRINTH write_serial_hex
#define PRINTLN write_serial_str("\n");

static uint64_t get_highest_usable_addr(struct limine_memmap_response* response) {

    if(response->entries == NULL) return 0;

    const uint64_t num_entries = response->entry_count;
    struct limine_memmap_entry** entries = response->entries;
    uint64_t largest_addr = 0;    
    
    PRINTS("Exploring RAM...");
    PRINTLN;
    PRINTS("Number of memory map entries: ");
    PRINTH(num_entries);
    PRINTLN;

    for(uint64_t i = 0; i < num_entries; i++) {
        struct limine_memmap_entry* current = entries[i];

        if(current == NULL) {
            PRINTS("Null entry. Skipping...\n");
            continue;
        }

        PRINTS("ptr: ");
        PRINTH((uint64_t)current);
        
        PRINTS(" | Base: ");
        PRINTH(current->base);        

        PRINTS(" | Length: ");
        PRINTH(current->length);

        PRINTS(" | Type: ");
        PRINTH(current->type); 

        PRINTLN;

        uint64_t ceiling = current->base + current->length;

        if(current->type == LIMINE_MEMMAP_USABLE && ceiling > largest_addr) 
            largest_addr = ceiling;
    }

    PRINTS("Largest usable address: ");
    PRINTH(largest_addr);
    PRINTLN;

    return largest_addr;
}

static void pmm_init(struct limine_memmap_response* response) {
    uint64_t highest_usable_addr = get_highest_usable_addr(response); 

}
