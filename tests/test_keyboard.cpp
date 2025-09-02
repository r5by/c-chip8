// tests/test_keyboard.cpp
#include <gtest/gtest.h>

extern "C" {
#include "keyboard.h"
#include "config.h"
#include "chip8_status.h"
}

TEST(Keyboard, DefaultZeroInitIsEnough) {
    Keyboard k{};  // all bytes zeroed

    for (int i = 0; i < NUM_KEYS; ++i) {
        bool down = true;
        ASSERT_EQ(CHIP8_OK, keyboard_is_down(&k, (uint8_t)i, &down));
        EXPECT_FALSE(down) << "key not zero-initialized: " << i;
    }

    uint8_t key = 0xAA;
    EXPECT_EQ(CHIP8_ERR_NO_KEY_PRESSED, keyboard_first_pressed(&k, &key));
    EXPECT_EQ(0xAA, key); // unchanged when none pressed
}

TEST(Keyboard, PressReleaseAndQuery) {
    Keyboard k{};

    // Press two keys
    EXPECT_EQ(CHIP8_OK, keyboard_press(&k, 0x1));
    EXPECT_EQ(CHIP8_OK, keyboard_press(&k, 0xA));

    bool down = false;
    EXPECT_EQ(CHIP8_OK, keyboard_is_down(&k, 0x1, &down));
    EXPECT_TRUE(down);
    EXPECT_EQ(CHIP8_OK, keyboard_is_down(&k, 0xA, &down));
    EXPECT_TRUE(down);

    // Release one key, the other remains pressed
    EXPECT_EQ(CHIP8_OK, keyboard_release(&k, 0x1));
    EXPECT_EQ(CHIP8_OK, keyboard_is_down(&k, 0x1, &down));
    EXPECT_FALSE(down);
    EXPECT_EQ(CHIP8_OK, keyboard_is_down(&k, 0xA, &down));
    EXPECT_TRUE(down);
}

TEST(Keyboard, FirstPressedScansInOrder) {
    Keyboard k{};

    // Press 0xC then 0x3; scanning 0..15 should report 0x3 first
    EXPECT_EQ(CHIP8_OK, keyboard_press(&k, 0xC));
    EXPECT_EQ(CHIP8_OK, keyboard_press(&k, 0x3));

    uint8_t key = 0xFF;
    ASSERT_EQ(CHIP8_OK, keyboard_first_pressed(&k, &key));
    EXPECT_EQ(0x3u, key);

    // Release 0x3; now 0xC is the first pressed
    EXPECT_EQ(CHIP8_OK, keyboard_release(&k, 0x3));
    ASSERT_EQ(CHIP8_OK, keyboard_first_pressed(&k, &key));
    EXPECT_EQ(0xCu, key);

    // Release 0xC; none pressed
    EXPECT_EQ(CHIP8_OK, keyboard_release(&k, 0xC));
    EXPECT_EQ(CHIP8_ERR_NO_KEY_PRESSED, keyboard_first_pressed(&k, &key));
    EXPECT_EQ(0xCu, key);  // unchanged
}

TEST(Keyboard, InvalidKeyIndicesReturnUnknownKeyError) {
    Keyboard k{};

    bool down = true;
    EXPECT_EQ(CHIP8_ERR_UNKNOWN_KEY_PRESSED, keyboard_press(&k,  (uint8_t)NUM_KEYS));
    EXPECT_EQ(CHIP8_ERR_UNKNOWN_KEY_PRESSED, keyboard_release(&k,(uint8_t)NUM_KEYS));
    EXPECT_EQ(CHIP8_ERR_UNKNOWN_KEY_PRESSED, keyboard_is_down(&k,(uint8_t)NUM_KEYS, &down));
    EXPECT_TRUE(down); // unchanged on error

    EXPECT_EQ(CHIP8_ERR_UNKNOWN_KEY_PRESSED, keyboard_press(&k, 0xFF));
    EXPECT_EQ(CHIP8_ERR_UNKNOWN_KEY_PRESSED, keyboard_is_down(&k, 0xFF, &down));
}
