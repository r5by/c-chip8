#ifndef CHIP8_REGS_H
#define CHIP8_REGS_H

#include "config.h"
#include <stdint.h>

typedef struct {
    uint8_t V[NUM_REGS];      // 16 general purpose 8-bit registers (V0-VF)
    
    uint16_t I;                     // 16-bit memory address register
    uint16_t PC;                    // 16-bit program counter
    uint8_t SP;                     // 8-bit stack pointer

    uint8_t DT;                     // 8-bit delay timer
    uint8_t ST;                     // 8-bit sound timer
} Registers;

void regs_init(Registers* regs);
void regs_reset(Registers* regs);

#endif // CHIP8_REGS_H
