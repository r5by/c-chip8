#include "mem.h"
#include <string.h>
#include <assert.h>

/* Compile-time guard: font must fit into memory at FONT_START_ADDR */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(FONT_START_ADDR + 80u <= MEMORY_SIZE, "fontset overflows memory");
#endif

/* Built-in font data (0-F) */
static const uint8_t fontset[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, /* 0 */
    0x20, 0x60, 0x20, 0x20, 0x70, /* 1 */
    0xF0, 0x10, 0xF0, 0x80, 0xF0, /* 2 */
    0xF0, 0x10, 0xF0, 0x10, 0xF0, /* 3 */
    0x90, 0x90, 0xF0, 0x10, 0x10, /* 4 */
    0xF0, 0x80, 0xF0, 0x10, 0xF0, /* 5 */
    0xF0, 0x80, 0xF0, 0x90, 0xF0, /* 6 */
    0xF0, 0x10, 0x20, 0x40, 0x40, /* 7 */
    0xF0, 0x90, 0xF0, 0x90, 0xF0, /* 8 */
    0xF0, 0x90, 0xF0, 0x10, 0xF0, /* 9 */
    0xF0, 0x90, 0xF0, 0x90, 0x90, /* A */
    0xE0, 0x90, 0xE0, 0x90, 0xE0, /* B */
    0xF0, 0x80, 0x80, 0x80, 0xF0, /* C */
    0xE0, 0x90, 0x90, 0x90, 0xE0, /* D */
    0xF0, 0x80, 0xF0, 0x80, 0xF0, /* E */
    0xF0, 0x80, 0xF0, 0x80, 0x80  /* F */
};

void memory_reset(Memory* m) {
    assert(m);
    memset(m->memory, 0, MEMORY_SIZE);
}

void memory_init(Memory* m) {
    assert(m);
    memory_reset(m);
    /* In case static assertion is not available, keep a debug assert too */
    assert((uint32_t)FONT_START_ADDR + (uint32_t)sizeof(fontset) <= (uint32_t)MEMORY_SIZE);
    memcpy(&m->memory[FONT_START_ADDR], fontset, sizeof(fontset));
}

Chip8Status memory_read(const Memory* m, uint16_t addr, uint8_t* out_value) {
    if (!m || !out_value) return CHIP8_ERR_NULL_ARG;
    if (addr >= MEMORY_SIZE) {
        return CHIP8_ERR_MEM_OOB;
    }
    *out_value = m->memory[addr];
    return CHIP8_OK;
}

Chip8Status memory_write(Memory* m, uint16_t addr, uint8_t value) {
    if (!m) return CHIP8_ERR_NULL_ARG;
    if (addr >= MEMORY_SIZE) {
        return CHIP8_ERR_MEM_OOB;
    }
    m->memory[addr] = value;
    return CHIP8_OK;
}

Chip8Status memory_load_rom(Memory* m, const uint8_t* data, size_t size) {
    if (!m || !data) return CHIP8_ERR_NULL_ARG;

    const size_t capacity = (size_t)MEMORY_SIZE - (size_t)PROGRAM_START_ADDRESS;
    if (size > capacity) {
        return CHIP8_ERR_ROM_TOO_LARGE;
    }
    memcpy(&m->memory[PROGRAM_START_ADDRESS], data, size);
    return CHIP8_OK;
}
