#ifndef CHIP8_MEM_H
#define CHIP8_MEM_H

#include <stdint.h>
#include <stdbool.h>
#include "config.h"

typedef struct {
    uint8_t memory[MEMORY_SIZE];
} Memory;

void memory_init(Memory* mem);
void memory_reset(Memory* mem);

uint8_t memory_read(Memory* mem, uint16_t address);
void memory_write(Memory* mem, uint16_t address, uint8_t value);

bool memory_load_rom(Memory* mem, const uint8_t* data, size_t size);

#endif // CHIP8_MEM_H
