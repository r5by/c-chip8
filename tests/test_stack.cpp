// tests/test_stack.cpp
#include <gtest/gtest.h>

// Use C++ to test C code: wrap C headers with extern "C"
extern "C" {
#include "stack.h"
#include "regs.h"
#include "config.h"
}

TEST(Stack, InitResetsStorageAndSP) {
    Stack s;
    Registers r;
    r.SP = 123; // non-zero to verify reset
    stack_init(&s, &r);

    // After init, SP should be zero and popping should underflow to 0
    EXPECT_EQ(0u, r.SP);
    EXPECT_EQ(0u, stack_pop(&s, &r));
}

TEST(Stack, PushPopLIFO_UpdatesSP) {
    Stack s;
    Registers r;
    stack_init(&s, &r);

    // Push two values; SP should track depth
    stack_push(&s, &r, 0x1234);
    EXPECT_EQ(1u, r.SP);
    stack_push(&s, &r, 0xABCD);
    EXPECT_EQ(2u, r.SP);

    // Pop in reverse order
    EXPECT_EQ(0xABCD, stack_pop(&s, &r));
    EXPECT_EQ(1u, r.SP);
    EXPECT_EQ(0x1234, stack_pop(&s, &r));
    EXPECT_EQ(0u, r.SP);

    // Empty -> underflow
    EXPECT_EQ(0u, stack_pop(&s, &r));
    EXPECT_EQ(0u, r.SP);
}

TEST(Stack, UnderflowReturnsZeroAndKeepsSPZero) {
    Stack s;
    Registers r;
    stack_init(&s, &r);

    EXPECT_EQ(0u, stack_pop(&s, &r));
    EXPECT_EQ(0u, r.SP);
    EXPECT_EQ(0u, stack_pop(&s, &r));
    EXPECT_EQ(0u, r.SP);
}

TEST(Stack, OverflowIsIgnoredAndSPClamped) {
    Stack s;
    Registers r;
    stack_init(&s, &r);

    // Fill to capacity with distinct values
    for (uint16_t i = 0; i < STACK_DEPTH; ++i) {
        stack_push(&s, &r, static_cast<uint16_t>(0x1000u + i));
    }
    EXPECT_EQ(STACK_DEPTH, r.SP);

    // Extra push beyond capacity should be ignored by policy
    stack_push(&s, &r, 0xFFFF);
    EXPECT_EQ(STACK_DEPTH, r.SP);

    // Pop all back; the last valid item should be 0x1000 + (STACK_DEPTH - 1)
    for (int i = STACK_DEPTH - 1; i >= 0; --i) {
        uint16_t expected = static_cast<uint16_t>(0x1000u + i);
        EXPECT_EQ(expected, stack_pop(&s, &r));
    }
    EXPECT_EQ(0u, r.SP);

    // Now empty: further pops underflow to 0
    EXPECT_EQ(0u, stack_pop(&s, &r));
    EXPECT_EQ(0u, r.SP);
}
