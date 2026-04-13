#ifndef DISPLAY_MU_H
#define DISPLAY_MU_H

#include <stdint.h>
#define DISPLAY_INIT_SUCCESS    (1<<6)

// Display API
uint8_t display_init(uint32_t *addr, uint32_t pitch, uint32_t width, uint32_t height);
void fb_clear();
void fb_putchar(char c);
void fb_print(const char *str);
void fb_print_hex(uint64_t val);
void fb_print_dec(uint64_t val);

#define PRINTS(msg)          fb_print(msg)
#define PRINTH(val)          fb_print_hex(val)
#define PRINTD(val)          fb_print_dec(val)
#define PRINTLN              fb_putchar('\n')
#define PRINTTAB             fb_putchar('\t')
#define PRINTF(msg, val)     { fb_print(msg); fb_putchar(' '); fb_print_dec(val); }

#endif