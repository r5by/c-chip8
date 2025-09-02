#ifndef INSTR_H
#define INSTR_H

#include <stdint.h>
#include "regs.h"
#include "mem.h"
#include "screen.h"
#include "stack.h"
#include "keyboard.h"

/* Instruction field helpers */
#define OP_NNN(op) ((uint16_t)((op) & 0x0FFF))          /* 12-bit addr   */
#define OP_X(op)   ((uint8_t)(((op) >> 8) & 0x0F))      /* X reg index   */
#define OP_Y(op)   ((uint8_t)(((op) >> 4) & 0x0F))      /* Y reg index   */
#define OP_KK(op)  ((uint8_t)((op) & 0x00FF))           /* 8-bit const   */
#define OP_N(op)   ((uint8_t)((op) & 0x000F))           /* low nibble    */

/* Execute a single opcode.
 * NOTE:
 *  - step (PC + 2) is delegated to caller of this function
 */
void exec(uint16_t opcode,
        Registers* regs,
        Memory* mem,
        Screen* screen,
        Stack* stack,
        Keyboard* keyboard);

#endif /* INSTR_H */
