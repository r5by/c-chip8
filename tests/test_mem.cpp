// tests/test_mem.cpp
#include <gtest/gtest.h>

extern "C" {
#include "mem.h"
#include "config.h"
#include "chip8_status.h"
}

TEST(Memory, InitClearsAndLoadsFont) {
    Memory m{};
    memory_init(&m);  // void API

    // 1) Region before program start is zeroed (spot checks)
    EXPECT_EQ(0u, m.memory[0]);
    EXPECT_EQ(0u, m.memory[PROGRAM_START_ADDRESS - 1]);

    // 2) Fontset content check: 16 glyphs × 5 bytes = 80 bytes
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
    for (int i = 0; i < 80; ++i) {
        EXPECT_EQ(expected_font[i], m.memory[FONT_START_ADDR + i]) << "mismatch at font byte " << i;
    }
}

TEST(Memory, ResetClearsAllBytes) {
    Memory m{};
    memory_init(&m);
    // Dirty a few locations
    m.memory[0] = 0xAA;
    m.memory[FONT_START_ADDR] = 0xBB;
    m.memory[PROGRAM_START_ADDRESS] = 0xCC;

    memory_reset(&m);

    // After reset, everything should be zero
    for (int i = 0; i < MEMORY_SIZE; ++i) {
        EXPECT_EQ(0u, m.memory[i]) << "non-zero at " << i;
    }
}

TEST(Memory, ReadWriteInsideBounds) {
    Memory m{}; memory_init(&m);

    // Write then read back
    EXPECT_EQ(CHIP8_OK, memory_write(&m, PROGRAM_START_ADDRESS, 0xAB));

    uint8_t v = 0;
    EXPECT_EQ(CHIP8_OK, memory_read(&m, PROGRAM_START_ADDRESS, &v));
    EXPECT_EQ(0xAB, v);
}

TEST(Memory, ReadOOBReturnsError) {
    Memory m{}; memory_init(&m);

    uint8_t v = 0xEE;
    EXPECT_EQ(CHIP8_ERR_MEM_OOB, memory_read(&m, MEMORY_SIZE, &v));  // exactly one past the end
    EXPECT_EQ(0xEE, v); // API shall NOT change param on OOB
}

TEST(Memory, WriteOOBReturnsError) {
    Memory m{}; memory_init(&m);

    EXPECT_EQ(CHIP8_ERR_MEM_OOB, memory_write(&m, MEMORY_SIZE, 0xFF));
    // Neighboring last valid byte should remain unchanged (still zero)
    EXPECT_EQ(0u, m.memory[MEMORY_SIZE - 1]);
}

TEST(Memory, LoadRomOkAndTooLarge) {
    Memory m{}; memory_init(&m);

    const uint8_t rom[8] = {1,2,3,4,5,6,7,8};
    EXPECT_EQ(CHIP8_OK, memory_load_rom(&m, rom, sizeof(rom)));

    // Verify payload placed at PROGRAM_START_ADDRESS
    for (size_t i = 0; i < sizeof(rom); ++i) {
        EXPECT_EQ(rom[i], m.memory[PROGRAM_START_ADDRESS + i]) << "mismatch at rom byte " << i;
    }

    // Oversized ROM should return error and not write past capacity
    const size_t too_big = (size_t)MEMORY_SIZE - (size_t)PROGRAM_START_ADDRESS + 1;
    std::vector<uint8_t> big(too_big, 0xEE);
    EXPECT_EQ(CHIP8_ERR_ROM_TOO_LARGE, memory_load_rom(&m, big.data(), big.size()));
}
