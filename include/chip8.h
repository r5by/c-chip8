#ifndef CHIP8_H
#define CHIP8_H

#include "config.h"
#include "mem.h"
#include "regs.h"
#include "stack.h"

struct Chip8 {
    // ram
    Memory chip8_mem;

    // CPU registers
    Registers chip8_regs;

    // stacks
    Stack chip8_stack;
    
};

#endif // CHIP8_H
