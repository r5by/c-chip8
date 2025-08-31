#include <string.h>   // memset
#include <assert.h>   // assert for sanity checks

#include "stack.h"
#include "regs.h"


/*
 * CHIP-8 uses a small return-address stack.
 * The stack storage lives in `Stack`, while the stack pointer (depth) lives in `Registers::SP`.
 * This allows the CPU state (registers) to own SP while the memory block holds the words.
 */

void stack_init(Stack* stack, Registers* regs) {
    assert(stack != NULL);
    assert(regs  != NULL);
    memset(stack->stack, 0, sizeof(stack->stack));
    regs->SP = 0;  // empty stack
}

void stack_reset(Stack* stack, Registers* regs) {
    // Alias to init; provided for API symmetry
    stack_init(stack, regs);
}

void stack_push(Stack* stack, Registers* regs, uint16_t value) {
    assert(stack != NULL);
    assert(regs  != NULL);

    if (regs->SP >= STACK_DEPTH) {
        // Overflow policy: ignore the push
        return;
    }
    stack->stack[regs->SP++] = value;
}

uint16_t stack_pop(Stack* stack, Registers* regs) {
    assert(stack != NULL);
    assert(regs  != NULL);

    if (regs->SP == 0) {
        // Underflow policy: return 0
        return 0;
    }
    return stack->stack[--regs->SP];
}