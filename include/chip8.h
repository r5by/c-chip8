#ifndef CHIP8_H
#define CHIP8_H

#include "config.h"
#include "mem.h"
#include "regs.h"
#include "stack.h"
#include "keyboard.h"

struct Chip8 {
    // ram
    Memory chip8_mem;

    // CPU registers
    Registers chip8_regs;

    // stacks
    Stack chip8_stack;

    // keyboard
    Keyboard chip8_kbd;
    
};

void chip8_init(struct Chip8 *chip8);

#endif // CHIP8_H
