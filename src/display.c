#include "headers/display.h"
#include "headers/font8x8_basic.h"

#define SCALE 1
// Terminal State
static uint32_t *fb_ptr = 0;
static uint32_t fb_pitch = 0;
static uint32_t fb_width = 0;
static uint32_t fb_height = 0;

static uint32_t cursor_x = 0;
static uint32_t cursor_y = 0;

uint8_t display_init(uint32_t *addr, uint32_t pitch, uint32_t width, uint32_t height) {
    fb_ptr = addr;
    fb_pitch = pitch;
    fb_width = width;
    fb_height = height;
    
    for (uint32_t i = 0; i < (pitch / 4) * height; i++) {
        fb_ptr[i] = 0x111111; 
    }

    return DISPLAY_INIT_SUCCESS;
}

void fb_putchar(char c) {
    if (!fb_ptr) return;

    uint32_t scale = SCALE; // 2 = 16x16, 3 = 24x24, 4 = 32x32

    uint32_t font_width = 8 * scale;
    uint32_t line_height = 10 * scale;

    if (c == '\n') {
        cursor_x = 0;
        cursor_y += line_height;
        return;
    }

    if (c == '\t') {
        cursor_x += font_width * 4; // 4 spaces
        return;
    }

    const char *bitmap = font8x8_basic[(uint8_t)c];
    uint32_t pixels_per_row = fb_pitch / 4;

    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            if ((bitmap[y] >> x) & 1) {
                for (uint32_t sy = 0; sy < scale; sy++) {
                    for (uint32_t sx = 0; sx < scale; sx++) {
                        uint32_t draw_y = cursor_y + (y * scale) + sy;
                        uint32_t draw_x = cursor_x + (x * scale) + sx;
                        if (draw_x < fb_width && draw_y < fb_height) {
                            uint32_t index = draw_y * pixels_per_row + draw_x;
                            fb_ptr[index] = 0xFFFFFF; // White
                        }
                    }
                }
            }
        }
    }
    cursor_x += font_width;

    if (cursor_x + font_width > fb_width) {
        cursor_x = 0;
        cursor_y += line_height;
    }
}

void fb_print(const char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        fb_putchar(str[i]);
    }
}

void fb_print_hex(uint64_t val) {
    char* hex_chars = "0123456789ABCDEF";

    fb_print("0x");
    for(int i = 15; i >= 0; i--) {
        uint8_t nibble = (val >> (i * 4)) & 0xf;
        fb_putchar(hex_chars[nibble]);
    }
}

void fb_print_dec(uint64_t val) {
    if (val == 0) {
        fb_putchar('0');
        return;
    }

    char buffer[20]; // 64-bit max is 20 digits long
    int i = 0;

    while (val > 0) {
        buffer[i++] = (val % 10) + '0';
        val /= 10;
    }

    while (i > 0) {
        fb_putchar(buffer[--i]);
    }
}

void fb_clear() {
    for (uint32_t y = 0; y < fb_height; y++) {
        for (uint32_t x = 0; x < fb_width; x++) {
            fb_ptr[y * (fb_pitch / 4) + x] = 0x00000000;
        }
    }    
    cursor_x = 0;
    cursor_y = 0;
}