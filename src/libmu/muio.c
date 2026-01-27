#define COM1 0x3f8
#define LINE_STATUS_REG (COM1 + 5)

// Send byte to hardware port
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %b0, %w1" :: "a"(val), "Nd"(port) : "memory");
}

// Receive byte to hardware port
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %w1, %b0" : "=a"(ret) : "Nd"(port));
    return ret;
}


static inline int is_transit_empty() {
    return inb(LINE_STATUS_REG) & 0x20;
}

static inline void write_serial(char a) {
    while(is_transit_empty() == 0);
    outb(COM1, a);
}

static inline void print_literal(const char* s) {
    for(int i = 0; s[i] != '\0'; i++) {
        write_serial(s[i]);
    }
    write_serial('\r');
    write_serial('\n');
}
