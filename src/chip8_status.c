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
        case CHIP8_ERR_NO_KEY_PRESSED:      return "No key pressed";
        case CHIP8_ERR_PIXEL_SET_FAILURE:   return "fail at setting pixel";
        case CHIP8_ERR_ROM_OPEN:            return "failed to open ROM file";
        case CHIP8_ERR_ROM_READ:            return "failed to read ROM";
        default:                            return "unknown";
    }
}
