#include "keyboard.h"

static inline bool key_in_bounds(uint8_t key) {
    return key < NUM_KEYS;
}

Chip8Status keyboard_press(Keyboard* kbd, uint8_t key) {
    CHIP8_CHECK_ARG(kbd);
    if (!key_in_bounds(key)) return CHIP8_ERR_UNKNOWN_KEY_PRESSED;

    kbd->state[key] = 1u; /* up->down; repeated down is fine */
    return CHIP8_OK;
}

Chip8Status keyboard_release(Keyboard* kbd, uint8_t key) {
    CHIP8_CHECK_ARG(kbd);
    if (!key_in_bounds(key)) return CHIP8_ERR_UNKNOWN_KEY_PRESSED;

    kbd->state[key] = 0u;
    return CHIP8_OK;
}

Chip8Status keyboard_is_down(const Keyboard* kbd, uint8_t key, bool* out_is_down) {
    CHIP8_CHECK_ARG(kbd);
    CHIP8_CHECK_ARG(out_is_down);
    if (!key_in_bounds(key))   return CHIP8_ERR_UNKNOWN_KEY_PRESSED;

    *out_is_down = (kbd->state[key] != 0u);
    return CHIP8_OK;
}

Chip8Status keyboard_first_pressed(const Keyboard* kbd, uint8_t* out_key) {
    CHIP8_CHECK_ARG(kbd);
    CHIP8_CHECK_ARG(out_key);

    for (uint8_t i = 0; i < NUM_KEYS; ++i) {
        if (kbd->state[i]) {
            *out_key = i;
            return CHIP8_OK;
        }
    }
    return CHIP8_ERR_STACK_UNDERFLOW; /* none pressed */
}
