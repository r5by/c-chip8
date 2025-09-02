// tests/test_instr.cpp
#include <gtest/gtest.h>

extern "C" {
#include "instr.h"
#include "mem.h"
#include "screen.h"
#include "stack.h"
#include "chip8_status.h"
#include "config.h"
#include "keyboard.h"
}

static void prestep_and_exec(uint16_t opcode,
                             Registers& regs,
                             Memory& mem,
                             Screen& scr,
                             Stack& stk,
                             Keyboard& kbd)
{
    regs.PC = static_cast<uint16_t>(regs.PC + 2); // caller pre-increments
    exec(opcode, &regs, &mem, &scr, &stk, &kbd);
}

/* ---------- 00E0: CLS ---------- */
TEST(Instr, CLS_ClearsScreen) {
    Memory m{};  memory_init(&m);
    Screen s{};  screen_init(&s);
    Stack  stk{};
    Keyboard kbd{};
    Registers r{}; r.PC = PROGRAM_START_ADDRESS;

    // Light a pixel first
    uint8_t sprite[1] = {0x80}; // 1000 0000
    (void)screen_draw_sprite(&s, 0, 0, sprite, 1);
    ASSERT_EQ(1u, screen_get_pixel(&s, 0, 0));

    prestep_and_exec(0x00E0, r, m, s, stk, kbd);
    EXPECT_EQ(0u, screen_get_pixel(&s, 0, 0));
}

/* ---------- 1nnn: JP addr ---------- */
TEST(Instr, JP_Absolute) {
    Memory m{};  memory_init(&m);
    Screen s{};  screen_init(&s);
    Stack  stk{};
    Keyboard kbd{};
    Registers r{}; r.PC = 0x300;

    prestep_and_exec(0x1234, r, m, s, stk, kbd);
    EXPECT_EQ(0x234, r.PC);
}

/* ---------- 2nnn / 00EE: CALL/RET ---------- */
TEST(Instr, CALL_RET_Roundtrip) {
    Memory m{};  memory_init(&m);
    Screen s{};  screen_init(&s);
    Stack  stk{};
    Keyboard kbd{};
    Registers r{}; r.PC = 0x400; r.SP = 0;

    // CALL 0x456: pushes return (0x402) and jumps
    prestep_and_exec(0x2456, r, m, s, stk, kbd);
    EXPECT_EQ(0x456, r.PC);
    EXPECT_GT(r.SP, 0); // pushed

    // simulate that RET is at 0x456; caller pre-increment does +2 to 0x458
    prestep_and_exec(0x00EE, r, m, s, stk, kbd);
    EXPECT_EQ(0x402, r.PC); // return to next after original call
}

/* Underflow RET keeps PC (already pre-incremented) */
TEST(Instr, RET_UnderflowKeepsPC) {
    Memory m{};  memory_init(&m);
    Screen s{};  screen_init(&s);
    Stack  stk{}; // empty
    Keyboard kbd{};
    Registers r{}; r.PC = 0x500; r.SP = 0;

    uint16_t pc_before = r.PC + 2;
    prestep_and_exec(0x00EE, r, m, s, stk, kbd);
    EXPECT_EQ(pc_before, r.PC);
}

/* ---------- 3xkk / 4xkk / 5xy0 / 9xy0 (skips) ---------- */
TEST(Instr, Skips_Compare) {
    Memory m{};  memory_init(&m);
    Screen s{};  screen_init(&s);
    Stack  stk{};
    Keyboard kbd{};
    Registers r{}; r.PC = 0x200;
    r.V[1] = 0xAB; r.V[2] = 0xAB; r.V[3] = 0x7F;

    // 3xkk: SE V1,0xAB -> skip
    prestep_and_exec(0x31AB, r, m, s, stk, kbd);
    EXPECT_EQ(0x204, r.PC); // incorrect, should be 0x204

    // Reset PC
    r.PC = 0x200;
    // 4xkk: SNE V3,0x80 (0x7F != 0x80) -> skip
    prestep_and_exec(0x4380, r, m, s, stk, kbd);
    EXPECT_EQ(0x204, r.PC);

    // 5xy0: SE V1,V2 (equal) -> skip
    r.PC = 0x200;
    prestep_and_exec(0x5120, r, m, s, stk, kbd);
    EXPECT_EQ(0x204, r.PC);

    // 9xy0: SNE V1,V2 (equal=false) -> no skip
    r.PC = 0x200;
    prestep_and_exec(0x9120, r, m, s, stk, kbd);
    EXPECT_EQ(0x202, r.PC);
}

/* ---------- 6xkk / 7xkk: LD, ADD ---------- */
TEST(Instr, LoadAndAddImmediate) {
    Memory m{};  memory_init(&m);
    Screen s{};  screen_init(&s);
    Stack  stk{};
    Keyboard kbd{};
    Registers r{}; r.PC = 0x200;

    prestep_and_exec(0x61FE, r, m, s, stk, kbd); // LD V1,0xFE
    EXPECT_EQ(0xFE, r.V[1]);

    prestep_and_exec(0x7105, r, m, s, stk, kbd); // ADD V1,0x05 -> 0x03 (wrap)
    EXPECT_EQ(0x03, r.V[1]);
}

/* ---------- 8xy* ALU ops ---------- */
TEST(Instr, ALU_Ops) {
    Memory m{};  memory_init(&m);
    Screen s{};  screen_init(&s);
    Stack  stk{};
    Keyboard kbd{};
    Registers r{}; r.PC = 0x200;
    r.V[1] = 0xF0; r.V[2] = 0x30;

    // 8xy4: V1 = V1 + V2; VF = carry
    prestep_and_exec(0x8124, r, m, s, stk, kbd);

    EXPECT_EQ((uint8_t)0x20, r.V[1]);  // 0xF0 + 0x30 = 0x120 -> 0x20 (8-bit wrap)
    EXPECT_EQ(1, r.V[0xF]);            // carry set
}

TEST(Instr, ALU_Ops_CarryBorrowShift) {
    Memory m{};  memory_init(&m);
    Screen s{};  screen_init(&s);
    Stack  stk{};
    Keyboard kbd{};
    Registers r{}; r.PC = 0x200;

    // ADD carry
    r.V[1] = 0xF0; r.V[2] = 0x30;
    prestep_and_exec(0x8124, r, m, s, stk, kbd); // V1 += V2
    EXPECT_EQ((uint8_t)0x20, r.V[1]);
    EXPECT_EQ(1, r.V[0xF]);

    // SUB not borrow: V1 - V2
    r.PC = 0x200; r.V[1] = 0x50; r.V[2] = 0x20;
    prestep_and_exec(0x8125, r, m, s, stk, kbd);
    EXPECT_EQ((uint8_t)0x30, r.V[1]);
    EXPECT_EQ(1, r.V[0xF]); // NOT borrow

    // SUBN not borrow: V1 = V2 - V1
    r.PC = 0x200; r.V[1] = 0x10; r.V[2] = 0x40;
    prestep_and_exec(0x8127, r, m, s, stk, kbd);
    EXPECT_EQ((uint8_t)0x30, r.V[1]);
    EXPECT_EQ(1, r.V[0xF]);

    // SHR V1 (LSB to VF)
    r.PC = 0x200; r.V[1] = 0x05;
    prestep_and_exec(0x8106, r, m, s, stk, kbd);
    EXPECT_EQ((uint8_t)0x02, r.V[1]);
    EXPECT_EQ(1, r.V[0xF]); // LSB was 1

    // SHL V1 (MSB to VF)
    r.PC = 0x200; r.V[1] = 0x80;
    prestep_and_exec(0x810E, r, m, s, stk, kbd);
    EXPECT_EQ((uint8_t)0x00, r.V[1]); // 0x100 -> 0 (8-bit wrap)
    EXPECT_EQ(1, r.V[0xF]); // MSB was 1
}

/* ---------- Annn / Bnnn / Fx1E ---------- */
TEST(Instr, LoadI_JumpV0_AddI) {
    Memory m{};  memory_init(&m);
    Screen s{};  screen_init(&s);
    Stack  stk{};
    Keyboard kbd{};
    Registers r{}; r.PC = 0x200;
    r.V[0] = 0x10;

    prestep_and_exec(0xAABC, r, m, s, stk, kbd);
    EXPECT_EQ(0xABC, r.I);

    prestep_and_exec(0xB100, r, m, s, stk, kbd); // JP V0, 0x100 -> 0x110
    EXPECT_EQ(0x110, r.PC);

    r.PC = 0x200; r.I = 0x0100; r.V[1] = 0x22;
    prestep_and_exec(0xF11E, r, m, s, stk, kbd); // ADD I, V1
    EXPECT_EQ(0x0122, r.I);
}

/* ---------- Cxkk: RND ---------- */
TEST(Instr, RandomMasked) {
    Memory m{};  memory_init(&m);
    Screen s{};  screen_init(&s);
    Stack  stk{};
    Keyboard kbd{};
    Registers r{}; r.PC = 0x200;

    srand(12345);
    // Compute expected using same expression
    uint8_t expected = (uint8_t)((rand() & 0xFF) & 0x3C);

    // reset seed so exec sees the same next rand()
    srand(12345);
    prestep_and_exec(0xC13C, r, m, s, stk, kbd); // RND V1, 0x3C
    EXPECT_EQ(expected, r.V[1]);
}

/* ---------- Fx07 / Fx15 / Fx18 ---------- */
TEST(Instr, TimersLoad) {
    Memory m{};  memory_init(&m);
    Screen s{};  screen_init(&s);
    Stack  stk{};
    Keyboard kbd{};
    Registers r{}; r.PC = 0x200;
    r.DT = 33; r.ST = 0;

    prestep_and_exec(0xF107, r, m, s, stk, kbd); // LD V1, DT
    EXPECT_EQ(33, r.V[1]);

    r.V[2] = 55;
    prestep_and_exec(0xF215, r, m, s, stk, kbd); // LD DT, V2
    EXPECT_EQ(55, r.DT);

    r.V[3] = 66;
    prestep_and_exec(0xF318, r, m, s, stk, kbd); // LD ST, V3
    EXPECT_EQ(66, r.ST);
}

/* ---------- Fx29: LD F, Vx ---------- */
TEST(Instr, FontAddress) {
    Memory m{};  memory_init(&m);
    Screen s{};  screen_init(&s);
    Stack  stk{};
    Keyboard kbd{};
    Registers r{}; r.PC = 0x200;

    r.V[4] = 0x0A; // digit 'A'
    prestep_and_exec(0xF429, r, m, s, stk, kbd);
    EXPECT_EQ((uint16_t)(FONT_START_ADDR + 5u * 0x0A), r.I);
}

/* ---------- Fx33: BCD ---------- */
TEST(Instr, BCDStore) {
    Memory m{};  memory_init(&m);
    Screen s{};  screen_init(&s);
    Stack  stk{};
    Keyboard kbd{};
    Registers r{}; r.PC = 0x200;
    r.I = 0x350;
    r.V[5] = 234;

    prestep_and_exec(0xF533, r, m, s, stk, kbd);
    uint8_t d0=0,d1=0,d2=0;
    ASSERT_EQ(CHIP8_OK, memory_read(&m, r.I + 0, &d0));
    ASSERT_EQ(CHIP8_OK, memory_read(&m, r.I + 1, &d1));
    ASSERT_EQ(CHIP8_OK, memory_read(&m, r.I + 2, &d2));
    EXPECT_EQ(2, d0);
    EXPECT_EQ(3, d1);
    EXPECT_EQ(4, d2);
}

/* ---------- Fx55 / Fx65: bulk transfer, I increments ---------- */
TEST(Instr, BulkStoreAndLoadWithIIncrement) {
    Memory m{};  memory_init(&m);
    Screen s{};  screen_init(&s);
    Stack  stk{};
    Keyboard kbd{};
    Registers r{}; r.PC = 0x200;

    // Prepare V0..V3
    r.V[0]=1; r.V[1]=2; r.V[2]=3; r.V[3]=4;
    r.I = 0x360;

    // Fx55: store V0..V3 at [I..I+3], then I += 4
    prestep_and_exec(0xF355, r, m, s, stk, kbd);
    uint8_t b=0;
    for (int i=0;i<=3;i++){
        ASSERT_EQ(CHIP8_OK, memory_read(&m, (uint16_t)(0x360 + i), &b));
        EXPECT_EQ((uint8_t)(i+1), b);
    }
    EXPECT_EQ((uint16_t)(0x360 + 4), r.I);

    // Change memory to known values, then Fx65 read back to V0..V3
    ASSERT_EQ(CHIP8_OK, memory_write(&m, r.I + 0, 10));
    ASSERT_EQ(CHIP8_OK, memory_write(&m, r.I + 1, 11));
    ASSERT_EQ(CHIP8_OK, memory_write(&m, r.I + 2, 12));
    ASSERT_EQ(CHIP8_OK, memory_write(&m, r.I + 3, 13));

    prestep_and_exec(0xF365, r, m, s, stk, kbd);
    EXPECT_EQ(10, r.V[0]);
    EXPECT_EQ(11, r.V[1]);
    EXPECT_EQ(12, r.V[2]);
    EXPECT_EQ(13, r.V[3]);
    EXPECT_EQ((uint16_t)(0x360 + 8), r.I); // I advanced another 4
}

/* ---------- Fx0A: wait for key ---------- */
TEST(Instr, Fx0A_WaitsWhenNoKey) {
    Memory m{}; memory_init(&m);
    Screen s{}; screen_init(&s);
    Stack  st{};
    Keyboard k{}; // all zero => no key pressed
    Registers r{}; r.PC = 0x200;

    // prestep +2 -> 0x202, Fx0A sees no key -> PC -= 2 => 0x200
    prestep_and_exec(0xF00A /* x=0 */, r, m, s, st, k);
    EXPECT_EQ(0x200, r.PC);
}

TEST(Instr, Fx0A_StoresKeyAndProceeds) {
    Memory m{}; memory_init(&m);
    Screen s{}; screen_init(&s);
    Stack  st{};
    Keyboard k{}; k.state[0xC] = 1; // pretend key 'C' pressed
    Registers r{}; r.PC = 0x200;

    // prestep +2 -> 0x202, Fx0A reads key 0xC, does not modify PC
    prestep_and_exec(0xF00A /* x=0 */, r, m, s, st, k);
    EXPECT_EQ(0x202, r.PC);
    EXPECT_EQ(0x0C, r.V[0]);
}
