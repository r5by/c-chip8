#ifndef CHIP8_STACK_H
#define CHIP8_STACK_H

#include <stdint.h>
#include "config.h"
#include "regs.h"

typedef struct {
    uint16_t stack[STACK_DEPTH];  // Stack with 16 levels
} Stack;

void stack_init(Stack* stack, Registers* regs);
void stack_reset(Stack* stack, Registers* regs);
void stack_push(Stack* stack, Registers* regs, uint16_t value);
uint16_t stack_pop(Stack* stack, Registers* regs);

#endif // CHIP8_STACK_H
