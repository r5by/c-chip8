// tests/test_mem.cpp
#include <gtest/gtest.h>

// Use C++ to test C code: wrap C headers with extern "C"
extern "C" {
#include "mem.h"
#include <string.h>
}

TEST(Memory, InitClearsAndLoadsFont) {
    Memory m;
    memory_init(&m);

    // 1) Check memory before program start is cleared (spot check)
    EXPECT_EQ(0u, m.memory[0]);
    EXPECT_EQ(0u, m.memory[PROGRAM_START_ADDRESS - 1]);

    // 2) Check fontset: 16 characters × 5 bytes = 80 bytes
    //    Compare with expected table (same as in mem.c)
    static const uint8_t expected_font[80] = {
        0xF0,0x90,0x90,0x90,0xF0, 0x20,0x60,0x20,0x20,0x70,
        0xF0,0x10,0xF0,0x80,0xF0, 0xF0,0x10,0xF0,0x10,0xF0,
        0x90,0x90,0xF0,0x10,0x10, 0xF0,0x80,0xF0,0x10,0xF0,
        0xF0,0x80,0xF0,0x90,0xF0, 0xF0,0x10,0x20,0x40,0x40,
        0xF0,0x90,0xF0,0x90,0xF0, 0xF0,0x90,0xF0,0x10,0xF0,
        0xF0,0x90,0xF0,0x90,0x90, 0xE0,0x90,0xE0,0x90,0xE0,
        0xF0,0x80,0x80,0x80,0xF0, 0xE0,0x90,0x90,0x90,0xE0,
        0xF0,0x80,0xF0,0x80,0xF0, 0xF0,0x80,0xF0,0x80,0x80
    };

    ASSERT_GE(MEMORY_SIZE, FONT_START_ADDR + 80);
    EXPECT_EQ(0, memcmp(&m.memory[FONT_START_ADDR], expected_font, sizeof(expected_font)));
}

TEST(Memory, ReadWriteInsideBounds) {
    Memory m; memory_init(&m);
    memory_write(&m, PROGRAM_START_ADDRESS, 0xAB);
    EXPECT_EQ(0xAB, memory_read(&m, PROGRAM_START_ADDRESS));
}

TEST(Memory, WriteOutOfBoundsIgnored) {
    Memory m; memory_init(&m);
    memory_write(&m, MEMORY_SIZE, 0xFF);            // Out of bounds (valid range 0..MEMORY_SIZE-1)
    EXPECT_NE(0xFF, m.memory[MEMORY_SIZE - 1]);     // Neighboring memory should not be corrupted
}

TEST(Memory, LoadRomOkAndTooLargeRejected) {
    Memory m; memory_init(&m);

    const uint8_t rom[8] = {1,2,3,4,5,6,7,8};
    EXPECT_TRUE(memory_load_rom(&m, rom, sizeof(rom)));
    EXPECT_EQ(0, memcmp(&m.memory[PROGRAM_START_ADDRESS], rom, sizeof(rom)));

    // Oversized ROM should return false and not write out of bounds
    std::vector<uint8_t> big(MEMORY_SIZE, 0xEE);
    EXPECT_FALSE(memory_load_rom(&m, big.data(), big.size()));
}
