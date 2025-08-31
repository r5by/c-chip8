#ifndef CHIP8_MEM_H
#define CHIP8_MEM_H

#include <stdint.h>
#include <stddef.h>
#include "config.h"
#include "chip8_status.h"

typedef struct {
    uint8_t memory[MEMORY_SIZE];
} Memory;

void memory_init (Memory* m);
void memory_reset(Memory* m);

Chip8Status memory_read     (const Memory* m, uint16_t addr, uint8_t* out_value);
Chip8Status memory_write    (Memory* m, uint16_t addr, uint8_t value);
Chip8Status memory_load_rom (Memory* m, const uint8_t* data, size_t size);

#endif /* CHIP8_MEM_H */
