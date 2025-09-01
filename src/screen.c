#include "screen.h"
#include <string.h> // memset

// Map (x, y) to linear index in row-major order with wrapping.
static inline size_t idx_wrap(uint8_t x, uint8_t y) {
    uint8_t _x = (uint8_t)(x % DISPLAY_WIDTH);
    uint8_t _y = (uint8_t)(y % DISPLAY_HEIGHT);
    return (size_t)_y * DISPLAY_WIDTH + (size_t)_x;
}

void screen_init(Screen* s) {
    if (!s) return;
    memset(s->pixels, 0, sizeof(s->pixels));
    s->dirty = true;
}

void screen_clear(Screen* s) {
    if (!s) return;
    memset(s->pixels, 0, sizeof(s->pixels));
    s->dirty = true;
}

uint8_t screen_get_pixel(const Screen* s, uint8_t x, uint8_t y) {
    if (!s) return 0;
    return s->pixels[idx_wrap(x, y)] ? 1u : 0u;
}

Chip8Status screen_set_pixel(Screen* s, uint8_t x, uint8_t y, uint8_t val) {
    CHIP8_CHECK_ARG(s);
    size_t i = idx_wrap(x, y);
    uint8_t newv = val ? 1u : 0u;

    if (s->pixels[i] != newv) {
        s->pixels[i] = newv;
        s->dirty = true;
        return CHIP8_OK;
    }

    return CHIP8_ERR_PIXEL_SET_FAILURE;
}

bool screen_toggle_pixel(Screen* s, uint8_t x, uint8_t y) {
    if (!s) return false;
    size_t i = idx_wrap(x, y);
    uint8_t before = s->pixels[i];
    s->pixels[i] ^= 1u;
    s->dirty = true;
    
    // Collision if a lit pixel got turned off due to XOR. (used in Dxyn instr)
    return (before == 1u && s->pixels[i] == 0u);
}

bool screen_draw_sprite(Screen* s, uint8_t x, uint8_t y, const uint8_t* sprite, uint8_t n) {
    if (!s || !sprite) return false;
    bool collision = false;

    for (uint8_t row = 0; row < n; ++row) {
        uint8_t bits = sprite[row];
        uint8_t _y = (uint8_t)(y + row);
        for (uint8_t col = 0; col < 8; ++col) {
            if ((bits & (uint8_t)(0x80u >> col)) == 0) continue;

            uint8_t _x = (uint8_t)(x + col);
            if (screen_toggle_pixel(s, _x, _y)) {
                collision = true;
            }
        }
    }
    
    return collision;
}

const uint8_t* screen_pixels(const Screen* s) {
    return s ? s->pixels : NULL;
}

bool screen_consume_dirty(Screen* s) {
    if (!s) return false;
    bool was_dirty = s->dirty;
    s->dirty = false;
    return was_dirty;
}
