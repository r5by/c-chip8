#ifndef SCREEN_H
#define SCREEN_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "config.h"
#include "chip8_status.h"

// Logical 1bpp screen buffer for CHIP-8 (64x32).
// Pixels are stored row-major as bytes 0/1.
typedef struct {
    uint8_t pixels[DISPLAY_WIDTH * DISPLAY_HEIGHT];
    bool    dirty;  // set true whenever any pixel changes
} Screen;

// Initialize the screen to all-black.
void screen_init(Screen* s);

// Clear the screen to black and mark dirty.
void screen_clear(Screen* s);

// Get pixel at (x, y) with wrapping. Returns 0 or 1.
uint8_t screen_get_pixel(const Screen* s, uint8_t x, uint8_t y);

// Set pixel at (x, y) to val (0/1) with wrapping.
Chip8Status screen_set_pixel(Screen* s, uint8_t x, uint8_t y, uint8_t val);

// Toggle (XOR) pixel at (x, y) with wrapping.
// Returns true if this operation caused a collision (1 -> 0).
bool screen_toggle_pixel(Screen* s, uint8_t x, uint8_t y);

// Draw an N-byte sprite located at memory `sprite` at (x, y).
// Each sprite byte encodes one row, MSB is the leftmost pixel.
// Returns true if any collision occurred (CHIP-8 VF semantics).
bool screen_draw_sprite(Screen* s, uint8_t x, uint8_t y, const uint8_t* sprite, uint8_t n);

// Get a const pointer to the raw pixel buffer.
const uint8_t* screen_pixels(const Screen* s);

// Consume and clear the "dirty" flag; returns whether it was dirty.
bool screen_consume_dirty(Screen* s);

#endif // SCREEN_H
