#include "instr.h"
#include "chip8_status.h"

void exec(uint16_t opcode,
        Registers* regs,
        Memory* mem,
        Screen* screen,
        Stack* stack)
{
    switch (opcode & 0xF000) {
        case 0x0000:
            switch (opcode) {
                case 0x00E0: /* CLS */
                    if (screen) {
                        screen_clear(screen);
                    } else {
                        CHIP8_LOG_WARN("CLS: screen is NULL");
                    }
                    break;

                case 0x00EE: /* RET */
                    //todo>
                    CHIP8_LOG_WARN("RET (00EE) not implemented yet");
                    break;

                default: {    /* 0nnn: SYS addr — 忽略 */
                    //todo>
                    break;
                }
            }
            break;

        default:
            CHIP8_LOG_WARN("Unimplemented opcode: 0x%04X", opcode);
            break;
    }
}
