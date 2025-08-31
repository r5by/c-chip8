// File: include/chip8.h
#ifndef CHIP8_H
#define CHIP8_H

#include "config.h"
#include "memory.h"

struct Chip8 {
    // CPU registers
    unsigned char V[16];  // General purpose registers V0-VF
    unsigned short I;     // Index register
    unsigned short pc;    // Program counter
    unsigned short sp;    // Stack pointer
    unsigned short stack[16]; // Stack
    
};

#endif // CHIP8_H
