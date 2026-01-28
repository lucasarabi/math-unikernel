// Headers included in kernel.c

#define PRINTS write_serial_str
#define PRINTH write_serial_hex
#define PRINTLN write_serial_str("\n");

static inline uint64_t get_highest_usable_addr(struct limine_memmap_response* response) {

    if(response->entries == NULL) return 0;

    const uint64_t num_entries = response->entry_count;
    struct limine_memmap_entry** entries = response->entries;
    uint64_t largest_addr = 0;    
    
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

        PRINTS(" | length : ");
        PRINTH(current->length);

        PRINTLN;

        uint64_t ceiling = current->base + current->length;

        if(ceiling > largest_addr) 
            largest_addr = ceiling;
    }
    return largest_addr;
}
