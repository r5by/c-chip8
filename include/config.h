#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

/* ============================
 * Emulator metadata
 * ============================ */
extern const char *EMULATOR_WINDOW_TITLE;

/* ============================
 * Window / Display configuration
 * ============================ */
#define EMULATOR_WINDOW_SCALER 10
#define DISPLAY_WIDTH          64
#define DISPLAY_HEIGHT         32

/* ============================
 * Memory configuration
 * ============================ */
enum {
    MEMORY_SIZE = 4096,             // ram size
    PROGRAM_START_ADDRESS = 0x200,  // program loader addr
    FONT_START_ADDR = 0x50          // fontset conventional start addr
};

/* ============================
 * Timing configuration
 * ============================ */
#define CPU_CLOCK_HZ   500  // Hz
#define TIMER_CLOCK_HZ 60   // Hz

/* ============================
 * Input configuration
 * ============================ */
#define NUM_KEYS 16

#endif // CONFIG_H
