#include <memory.h>
#include "chip8.h"

void chip8_init(struct Chip8 *chip8) {
    // Initialize the chip8 emulator
    // Set up memory, registers, and other necessary components
    // ...
    memset(chip8, 0, sizeof(struct Chip8));
    memory_init(&chip8->chip8_mem);
    screen_init(&chip8->chip8_disp);
}