#ifndef CHIP8_KEYBOARD_H
#define CHIP8_KEYBOARD_H

#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#include "chip8_status.h"

/*
 * CHIP-8 16-key hexadecimal keypad (0x0..0xF).
 * Minimal state-only model: no event queue, just pressed states.
 */
typedef struct {
    /* 0 = up, 1 = down; index is the CHIP-8 key value (0x0..0xF). */
    uint8_t state[NUM_KEYS];
} Keyboard;

/* Set key 'down'. Repeated presses while already down are no-ops. */
Chip8Status keyboard_press  (Keyboard* kbd, uint8_t key);

/* Set key 'up'. Releasing a key that is already up is harmless. */
Chip8Status keyboard_release(Keyboard* kbd, uint8_t key);

/* Query a single key's state. */
Chip8Status keyboard_is_down(const Keyboard* kbd, uint8_t key, bool* out_is_down);

/*
 * Find the first pressed key by scanning 0..15 (used by Fx0A).
 * On success: returns CHIP8_OK and writes the key into out_key.
 * If no key is pressed: returns CHIP8_ERR_STACK_UNDERFLOW and does NOT modify out_key.
 */
Chip8Status keyboard_first_pressed(const Keyboard* kbd, uint8_t* out_key);

#endif /* CHIP8_KEYBOARD_H */
