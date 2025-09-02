#include <string.h>   // memset
#include <assert.h>

#include "stack.h"
#include "regs.h"
#include "chip8_status.h"

/*
 * CHIP-8 uses a small return-address stack.
 * The stack storage lives in `Stack`, while the stack pointer (depth) lives in `Registers::SP`.
 * Functions return Chip8Status for uniform error handling and optional logging.
 */

Chip8Status stack_init(Stack* stack, Registers* regs) {
    CHIP8_CHECK_ARG(stack);
    CHIP8_CHECK_ARG(regs);

    memset(stack->stack, 0, sizeof(stack->stack));
    regs->SP = 0;  // empty stack
    return CHIP8_OK;
}

Chip8Status stack_reset(Stack* stack, Registers* regs) {
    return stack_init(stack, regs);
}

Chip8Status stack_push(Stack* stack, Registers* regs, uint16_t value) {
    CHIP8_CHECK_ARG(stack);
    CHIP8_CHECK_ARG(regs);

    if (regs->SP >= STACK_DEPTH) {
        CHIP8_LOG_ERROR("stack overflow (SP=%u, cap=%u)",
                        (unsigned)regs->SP, (unsigned)STACK_DEPTH);
        return CHIP8_ERR_STACK_OVERFLOW;
    }
    stack->stack[regs->SP++] = value;
    return CHIP8_OK;
}

Chip8Status stack_pop(Stack* stack, Registers* regs, uint16_t* out_value) {
    CHIP8_CHECK_ARG(stack);
    CHIP8_CHECK_ARG(regs);
    CHIP8_CHECK_ARG(out_value);

    if (regs->SP == 0) {
        CHIP8_LOG_WARN("stack underflow on pop");
        // *out_value = 0;  //don't care out value (register)
        return CHIP8_ERR_STACK_UNDERFLOW;
    }
    *out_value = stack->stack[--regs->SP];
    return CHIP8_OK;
}
