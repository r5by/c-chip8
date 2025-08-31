#include "chip8_status.h"

const char* chip8_status_str(Chip8Status s) {
    switch (s) {
        case CHIP8_OK:                      return "OK";
        case CHIP8_ERR_NULL_ARG:            return "NULL argument";
        case CHIP8_ERR_STACK_OVERFLOW:      return "stack overflow";
        case CHIP8_ERR_STACK_UNDERFLOW:     return "stack underflow";
        case CHIP8_ERR_MEM_OOB:             return "memory out-of-bounds";
        case CHIP8_ERR_ROM_TOO_LARGE:       return "ROM too large";
        case CHIP8_ERR_UNKNOWN_KEY_PRESSED: return "unknown key pressed";
        default:                            return "unknown";
    }
}
