#ifndef CHIP8_STATUS_H
#define CHIP8_STATUS_H

#include <stdint.h>
#include <stdio.h>

/* Unified status codes for the emulator core. */
typedef enum Chip8Status {
    CHIP8_OK = 0,
    CHIP8_ERR_NULL_ARG,
    CHIP8_ERR_STACK_OVERFLOW,
    CHIP8_ERR_STACK_UNDERFLOW,
    CHIP8_ERR_MEM_OOB,           /* memory out-of-bounds */
    CHIP8_ERR_ROM_TOO_LARGE,
    CHIP8_ERR_UNKNOWN_KEY_PRESSED,     /* invalid keypad index (not 0..15) */
    CHIP8_ERR_NO_KEY_PRESSED,
    CHIP8_ERR_PIXEL_SET_FAILURE,
    CHIP8_ERR_ROM_OPEN,            /* failed to open ROM file */
    CHIP8_ERR_ROM_READ,            /* failed to read ROM */   
} Chip8Status;

/* Convert status to a short, stable string. */
const char* chip8_status_str(Chip8Status s);

/* Optional logging helpers: compiled out unless CHIP8_ENABLE_LOG is defined. */
#ifdef CHIP8_ENABLE_LOG
  #define CHIP8_LOG_ERROR(fmt, ...) \
      do { fprintf(stderr, "[chip8][error] " fmt "\n", __VA_ARGS__); } while (0)
  #define CHIP8_LOG_WARN(fmt, ...) \
      do { fprintf(stderr, "[chip8][warn ] " fmt "\n", __VA_ARGS__); } while (0)
#else
  #define CHIP8_LOG_ERROR(fmt, ...) do { (void)0; } while (0)
  #define CHIP8_LOG_WARN(fmt, ...)  do { (void)0; } while (0)
#endif

/* Small guard macros for early returns. */
#define CHIP8_CHECK_ARG(p) \
    do { if (!(p)) return CHIP8_ERR_NULL_ARG; } while (0)

#endif /* CHIP8_STATUS_H */
