#define COM1 0x3f8

// Send byte to hardware port
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %b0, %w1" 
                        : 
                        : "a"(val), "Nd"(port) 
                        : "memory"
                    );
}

static inline void print_literal(const char* string_literal) {
    char* char_index = string_literal;
    while(*char_index != '\0') {
        outb(COM1, *char_index);
        char_index++;
    }
    outb(COM1, '\n');
}
