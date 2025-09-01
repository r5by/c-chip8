#ifndef CHIP8_H
#define CHIP8_H

#include "config.h"
#include "mem.h"
#include "regs.h"
#include "stack.h"
#include "keyboard.h"
#include "screen.h"

struct Chip8 {
    // ram
    Memory chip8_mem;

    // CPU registers
    Registers chip8_regs;

    // stacks
    Stack chip8_stack;

    // keyboard
    Keyboard chip8_kbd;

    // display
    Screen chip8_disp;
};

void chip8_init(struct Chip8 *c8);
Chip8Status chip8_load_rom(struct Chip8* c8, const char* filepath);
Chip8Status chip8_step(struct Chip8* c8);

void dump_n(const struct Chip8* c8,
                  uint16_t start_addr,
                  size_t   nbytes,
                  int      words_per_line);
#define mem_dump(c8, start_addr, nbytes) \
    dump_n((c8), (start_addr), (nbytes), 4) /* 1 word == 2 bytes, 4 words per line by default */

#endif // CHIP8_H
