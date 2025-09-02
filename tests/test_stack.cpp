// tests/test_stack.cpp
#include <gtest/gtest.h>

extern "C" {
#include "stack.h"
#include "regs.h"
#include "config.h"
#include "chip8_status.h"
}

TEST(Stack, InitResetsStorageAndSP) {
    Stack s{};
    Registers r{};
    r.SP = 123;  // non-zero to verify reset

    EXPECT_EQ(CHIP8_OK, stack_init(&s, &r));
    EXPECT_EQ(0u, r.SP);

    uint16_t out = 0xFFFF;
    // Pop on empty should underflow and MUST NOT change 'out'
    EXPECT_EQ(CHIP8_ERR_STACK_UNDERFLOW, stack_pop(&s, &r, &out));
    EXPECT_EQ(0xFFFFu, out);
    EXPECT_EQ(0u, r.SP);
}

TEST(Stack, PushPopLIFO_UpdatesSPAndReturnsStatus) {
    Stack s{};
    Registers r{};
    ASSERT_EQ(CHIP8_OK, stack_init(&s, &r));

    // Push two values; SP should track depth
    EXPECT_EQ(CHIP8_OK, stack_push(&s, &r, 0x1234));
    EXPECT_EQ(1u, r.SP);
    EXPECT_EQ(CHIP8_OK, stack_push(&s, &r, 0xABCD));
    EXPECT_EQ(2u, r.SP);

    // Pop in reverse order
    uint16_t v = 0;
    EXPECT_EQ(CHIP8_OK, stack_pop(&s, &r, &v));
    EXPECT_EQ(0xABCD, v);
    EXPECT_EQ(1u, r.SP);

    EXPECT_EQ(CHIP8_OK, stack_pop(&s, &r, &v));
    EXPECT_EQ(0x1234, v);
    EXPECT_EQ(0u, r.SP);

    // Empty -> underflow; 'v' must remain unchanged
    v = 0xDEAD;
    EXPECT_EQ(CHIP8_ERR_STACK_UNDERFLOW, stack_pop(&s, &r, &v));
    EXPECT_EQ(0xDEADu, v);
    EXPECT_EQ(0u, r.SP);
}

TEST(Stack, UnderflowReturnsErrorAndKeepsSPZero) {
    Stack s{};
    Registers r{};
    ASSERT_EQ(CHIP8_OK, stack_init(&s, &r));

    uint16_t v = 1234;
    EXPECT_EQ(CHIP8_ERR_STACK_UNDERFLOW, stack_pop(&s, &r, &v));
    EXPECT_EQ(1234u, v);
    EXPECT_EQ(0u, r.SP);

    // Repeated underflow still must not touch 'v'
    EXPECT_EQ(CHIP8_ERR_STACK_UNDERFLOW, stack_pop(&s, &r, &v));
    EXPECT_EQ(1234u, v);
    EXPECT_EQ(0u, r.SP);
}

TEST(Stack, OverflowIsReportedAndSPClamped) {
    Stack s{};
    Registers r{};
    ASSERT_EQ(CHIP8_OK, stack_init(&s, &r));

    // Fill to capacity with distinct values
    for (uint16_t i = 0; i < STACK_DEPTH; ++i) {
        ASSERT_EQ(CHIP8_OK, stack_push(
            &s, &r, static_cast<uint16_t>(0x1000u + i)));
    }
    EXPECT_EQ(STACK_DEPTH, r.SP);

    // One more push triggers overflow error and does not change SP
    EXPECT_EQ(CHIP8_ERR_STACK_OVERFLOW, stack_push(&s, &r, 0xFFFF));
    EXPECT_EQ(STACK_DEPTH, r.SP);

    // Pop all back and verify order
    for (int i = STACK_DEPTH - 1; i >= 0; --i) {
        uint16_t expected = static_cast<uint16_t>(0x1000u + i);
        uint16_t v = 0;
        EXPECT_EQ(CHIP8_OK, stack_pop(&s, &r, &v));
        EXPECT_EQ(expected, v);
    }
    EXPECT_EQ(0u, r.SP);

    // Now empty: further pop underflows and must not touch 'v'
    uint16_t v = 0xBEEF;
    EXPECT_EQ(CHIP8_ERR_STACK_UNDERFLOW, stack_pop(&s, &r, &v));
    EXPECT_EQ(0xBEEFu, v);
    EXPECT_EQ(0u, r.SP);
}
