#include "instr.h"
#include "chip8_status.h"
#include "config.h"      // MEMORY_SIZE, FONT_START_ADDR
#include <stdlib.h>      // rand()

#define VF (regs->V[0xF])

void exec(uint16_t op,
          Registers* regs,
          Memory* mem,
          Screen* screen,
          Stack* stack,
          Keyboard* kbd)
{
    const uint8_t  x   = OP_X(op);
    const uint8_t  y   = OP_Y(op);
    const uint8_t  kk  = OP_KK(op);
    const uint8_t  n   = OP_N(op);
    const uint16_t nnn = OP_NNN(op);

    switch (op & 0xF000) {

    case 0x0000:
        switch (op) {
        case 0x00E0: { // CLS
            if (!screen) CHIP8_LOG_WARN("CLS: screen is NULL");
            screen_clear(screen);
            break;
        }
        case 0x00EE: { // RET
            Chip8Status st = stack_pop(stack, regs, &regs->PC);
            if (st != CHIP8_OK) {
                CHIP8_LOG_ERROR("RET failed: %s (PC stays at 0x%03X)",
                                chip8_status_str(st), regs->PC);
            }
            break;
        }
        default:
            /* 0nnn: SYS addr — ignored for modern interpreters */
            break;
        }
        break;

    case 0x1000: { // 1nnn: JP addr
        regs->PC = nnn;
        break;
    }

    case 0x2000: { // 2nnn: CALL addr
        Chip8Status st = stack_push(stack, regs, regs->PC); // caller pre-incremented
        if (st != CHIP8_OK) {
            CHIP8_LOG_ERROR("CALL push failed: %s", chip8_status_str(st));
            break;
        }
        regs->PC = nnn;
        break;
    }

    case 0x3000: { // 3xkk: SE Vx, byte
        if (regs->V[x] == kk) regs->PC += 2;
        break;
    }

    case 0x4000: { // 4xkk: SNE Vx, byte
        if (regs->V[x] != kk) regs->PC += 2;
        break;
    }

    case 0x5000: { // 5xy0: SE Vx, Vy
        if (n != 0x0)  {
            CHIP8_LOG_WARN("Unimplemented opcode: 0x%04X", op);
            break;
        }

        if (regs->V[x] == regs->V[y]) regs->PC += 2;
        break;
    }

    case 0x6000: { // 6xkk: LD Vx, byte
        regs->V[x] = kk;
        break;
    }

    case 0x7000: { // 7xkk: ADD Vx by kk, byte
        regs->V[x] += kk;
        break;
    }

    case 0x8000:
        switch (n) {
        case 0x0: // 8xy0: LD Vx, Vy
            regs->V[x] = regs->V[y];
            break;
        case 0x1: // 8xy1: OR Vx, Vy
            regs->V[x] |= regs->V[y]; VF = 0; break;
        case 0x2: // 8xy2: AND Vx, Vy
            regs->V[x] &= regs->V[y]; VF = 0; break;
        case 0x3: // 8xy3: XOR Vx, Vy
            regs->V[x] ^= regs->V[y]; VF = 0; break;
        case 0x4: { // 8xy4: ADD Vx, Vy (with carry)
            uint16_t sum = (uint16_t)regs->V[x] + (uint16_t)regs->V[y];
            VF = (sum > 0xFF) ? 1 : 0;
            regs->V[x] = (uint8_t)(sum & 0xFF);
            break;
        }
        case 0x5: { // 8xy5: SUB Vx, Vy (Vx = Vx - Vy)
            VF = (regs->V[x] > regs->V[y]) ? 1 : 0; // NOT borrow
            regs->V[x] = (uint8_t)(regs->V[x] - regs->V[y]);
            break;
        }
        case 0x6: { // 8xy6: SHR Vx (VF = LSB of Vx)
            VF = (uint8_t)(regs->V[x] & 0x01);
            regs->V[x] >>= 1;
            break;
        }
        case 0x7: { // 8xy7: SUBN Vx, Vy (Vx = Vy - Vx)
            VF = (regs->V[y] > regs->V[x]) ? 1 : 0; // NOT borrow
            regs->V[x] = (uint8_t)(regs->V[y] - regs->V[x]);
            break;
        }
        case 0xE: { // 8xyE: SHL Vx (VF = MSB of Vx)
            VF = (uint8_t)((regs->V[x] & 0x80) ? 1 : 0);
            regs->V[x] = (uint8_t)(regs->V[x] << 1);
            break;
        }
        default:
            CHIP8_LOG_WARN("Unimplemented opcode: 0x%04X", op);
            break;
        }
        break;

    case 0x9000: { // 9xy0: SNE Vx, Vy
        if (n != 0x0) {
            CHIP8_LOG_WARN("Unimplemented opcode: 0x%04X", op);
            break;
        }
        if (regs->V[x] != regs->V[y]) regs->PC += 2;
        break;
    }

    case 0xA000: { // Annn: LD I, addr
        regs->I = nnn;
        break;
    }

    case 0xB000: { // Bnnn: JP V0, addr
        regs->PC = (uint16_t)(nnn + regs->V[0]);
        break;
    }

    case 0xC000: { // Cxkk: RND Vx, byte
        regs->V[x] = (uint8_t)((rand() & 0xFF) & kk);
        break;
    }

    case 0xD000: { // Dxyn: DRW Vx, Vy, nibble
        if (!screen) { CHIP8_LOG_ERROR("DRW: screen is NULL"); break; }
        if (n == 0) { VF = 0; break; } // standard CHIP-8: n==0 draws 0 rows

        const uint16_t I = regs->I;
        if (I >= MEMORY_SIZE) {
            CHIP8_LOG_ERROR("DRW: I out of bounds: 0x%03X", I);
            VF = 0;
            break;
        }

        // Clamp rows so we never read past RAM end; well-formed ROMs keep I+n in bounds.
        uint8_t rows = n;
        size_t max_rows = (size_t)MEMORY_SIZE - (size_t)I;
        if ((size_t)rows > max_rows) {
            rows = (uint8_t)max_rows;
            CHIP8_LOG_WARN("DRW: sprite truncated at RAM end (I=0x%03X, n=%u -> rows=%u)", I, n, rows);
        }

        const uint8_t* sprite = &mem->memory[I];
        bool collision = screen_draw_sprite(screen, regs->V[x], regs->V[y], sprite, rows);
        VF = collision ? 1 : 0;
        break;
    }

    case 0xE000: { // Ex9E / ExA1
        bool down = false;
        Chip8Status st = keyboard_is_down(kbd, regs->V[x], &down);
        if (st != CHIP8_OK) {
            // Out-of-range key or null ptr: do not skip; just log
            CHIP8_LOG_ERROR("SKP/SKNP key check failed: %s (Vx=0x%02X)",
                            chip8_status_str(st), regs->V[x]);
            break;
        }

        switch (kk) {
        case 0x9E: // SKP Vx: skip if key(Vx) is pressed
            if (down) { regs->PC += 2; }
            break;

        case 0xA1: // SKNP Vx: skip if key(Vx) is NOT pressed
            if (!down) { regs->PC += 2; }
            break;

        default:
            CHIP8_LOG_WARN("Unimplemented opcode: 0x%04X", op);
            break;
        }
        
        break;
    }

    case 0xF000:
        switch (kk) {
        case 0x07: // Fx07: LD Vx, DT
            regs->V[x] = regs->DT;
            break;

        case 0x0A: // Fx0A: LD Vx, K — wait for key
            if (!kbd) {
                CHIP8_LOG_ERROR("Fx0A: Keyboard is NULL");
                regs->PC -= 2;   // hold on this opcode
                break;
            }

            uint8_t key = 0;
            Chip8Status st = keyboard_first_pressed(kbd, &key);
            if (st != CHIP8_OK) {
                CHIP8_LOG_ERROR("Fx0A: key check failed: %s", chip8_status_str(st));
                regs->PC -= 2;   // fail-safe: hold here
            }

            regs->V[x] = key;   // store the key index 0..15
            // caller already pre-incremented PC; just continue
            break;

        case 0x15: // Fx15: LD DT, Vx
            regs->DT = regs->V[x];
            break;

        case 0x18: // Fx18: LD ST, Vx
            regs->ST = regs->V[x];
            break;

        case 0x1E: { // Fx1E: ADD I, Vx
            regs->I += regs->V[x];
            // VF unaffected in original Chip-8
            break;
        }

        case 0x29: { // Fx29: LD F, Vx (font sprite address)
            uint8_t digit = (uint8_t)(regs->V[x] & 0x0F);
            regs->I = (uint16_t)(FONT_START_ADDR + digit * DEFAULT_SPRITE_HIGHT);
            break;
        }

        case 0x33: { // Fx33: BCD of Vx at [I..I+2]
            uint16_t I = regs->I;
            uint8_t v  = regs->V[x]; // v = v0*100 + v1*10 + v2
            Chip8Status v0 = memory_write(mem, I + 0, (uint8_t)(v / 100));
            Chip8Status v1 = memory_write(mem, I + 1, (uint8_t)((v / 10) % 10));
            Chip8Status v2 = memory_write(mem, I + 2, (uint8_t)(v % 10));
            if (v0 != CHIP8_OK || v1 != CHIP8_OK || v2 != CHIP8_OK) {
                CHIP8_LOG_ERROR("Fx33 write OOB at I=0x%03X", I);
            }
            break;
        }

        case 0x55: { // Fx55: LD [I], V0..Vx
            uint16_t I = regs->I;
            bool ok = true;
            for (uint8_t i = 0; i <= x; ++i) {
                if (memory_write(mem, (uint16_t)(I + i), regs->V[i]) != CHIP8_OK) {
                    CHIP8_LOG_ERROR("Fx55 OOB at I+%u (I=0x%03X)", (unsigned)i, I);
                    ok = false; break;
                }
            }
            // Original CHIP-8 increments I; keep I unchanged on error
            if (ok) regs->I = (uint16_t)(I + x + 1);
            break;
        }

        case 0x65: { // Fx65: LD V0..Vx, [I]
            uint16_t I = regs->I;
            bool ok = true;
            for (uint8_t i = 0; i <= x; ++i) {
                uint8_t v = 0;
                if (memory_read(mem, (uint16_t)(I + i), &v) != CHIP8_OK) {
                    CHIP8_LOG_ERROR("Fx65 OOB at I+%u (I=0x%03X)", (unsigned)i, I);
                    ok = false; break;
                }
                regs->V[i] = v;
            }
            if (ok) regs->I = (uint16_t)(I + x + 1); // Original CHIP-8 increments I
            break;
        }

        default:
            CHIP8_LOG_WARN("Unimplemented opcode: 0x%04X", op);
            break;
        }
        break;

    default:
        CHIP8_LOG_WARN("Unimplemented opcode: 0x%04X", op);
        break;
    }
}
