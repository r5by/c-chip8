// include/stack.h
#ifndef CHIP8_STACK_H
#define CHIP8_STACK_H

#include <stdint.h>
#include "config.h"
#include "chip8_status.h"
#include "regs.h"

/* Return-address stack. The storage lives here; SP (depth) lives in Registers. */
typedef struct {
    uint16_t stack[STACK_DEPTH];
} Stack;

/* All APIs return Chip8Status; stack_pop outputs the value via out param. */
Chip8Status stack_init (Stack* stack, Registers* regs);
Chip8Status stack_reset(Stack* stack, Registers* regs);
Chip8Status stack_push (Stack* stack, Registers* regs, uint16_t value);
Chip8Status stack_pop  (Stack* stack, Registers* regs, uint16_t* out_value);

#endif /* CHIP8_STACK_H */
