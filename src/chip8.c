#include <stdio.h>    // FILE, fopen, fread, fseek, ftell
#include <stdlib.h>   // malloc, free
#include <string.h>   // memset

#include "chip8.h"

void chip8_init(struct Chip8 *chip8) {
    // Initialize the chip8 emulator
    // Set up memory, registers, and other necessary components
    // ...
    memset(chip8, 0, sizeof(struct Chip8));
    memory_init(&chip8->chip8_mem);
    screen_init(&chip8->chip8_disp);
}

/* Helper: get file size (returns 0 on failure). */
static size_t file_size(FILE* f) {
    if (fseek(f, 0, SEEK_END) != 0) return 0;
    long sz = ftell(f);
    if (sz < 0) return 0;
    if (fseek(f, 0, SEEK_SET) != 0) return 0;
    return (size_t)sz;
}

Chip8Status chip8_load_rom(struct Chip8* chip8, const char* filepath) {
    CHIP8_CHECK_ARG(chip8);
    CHIP8_CHECK_ARG(filepath);

    FILE* fp = NULL;

#ifdef _MSC_VER
    if (fopen_s(&fp, filepath, "rb") != 0 || !fp) {
        CHIP8_LOG_ERROR("Failed to open ROM: %s", filepath);
        return CHIP8_ERR_ROM_OPEN;
    }
#else
    fp = fopen(filepath, "rb");
    if (!fp) {
        CHIP8_LOG_ERROR("Failed to open ROM: %s", filepath);
        return CHIP8_ERR_ROM_OPEN;
    }
#endif

    size_t sz = file_size(fp);
    if (sz == 0) {
        fclose(fp);
        CHIP8_LOG_ERROR("ROM is empty or unreadable: %s", filepath);
        return CHIP8_ERR_ROM_READ;
    }

    uint8_t* buf = (uint8_t*)malloc(sz);
    if (!buf) {
        fclose(fp);
        CHIP8_LOG_ERROR("Out of memory reading ROM: %zu bytes", sz);
        return CHIP8_ERR_ROM_READ;
    }

    size_t n = fread(buf, 1, sz, fp);
    fclose(fp);
    if (n != sz) {
        free(buf);
        CHIP8_LOG_ERROR("Short read: got=%zu, expected=%zu", n, sz);
        return CHIP8_ERR_ROM_READ;
    }

    Chip8Status st = memory_load_rom(&chip8->chip8_mem, buf, sz);
    free(buf);
    if (st != CHIP8_OK) {
        CHIP8_LOG_ERROR("memory_load_rom failed: %s", chip8_status_str(st));
        return st;
    }

    return CHIP8_OK;
}

void dump_n(const struct Chip8* c8,
                  uint16_t start_addr,
                  size_t   nbytes,
                  int      words_per_line)
{
    if (!c8 || nbytes == 0) return;
    if (words_per_line <= 0) words_per_line = 4;

    const uint8_t* mem = c8->chip8_mem.memory;

    size_t start = (size_t)start_addr;
    if (start >= MEMORY_SIZE) return;

    size_t end   = start + nbytes;
    if (end > MEMORY_SIZE) end = MEMORY_SIZE;

    const size_t line_bytes = (size_t)words_per_line * 2u;

    for (size_t line_addr = start; line_addr < end; line_addr += line_bytes) {
        printf("0x%03X:", (unsigned)(line_addr & 0xFFFu));

        for (int w = 0; w < words_per_line; ++w) {
            size_t wa = line_addr + (size_t)w * 2u;

            if (wa + 1 < end) {
                // big-endian
                uint8_t hi = mem[wa];
                uint8_t lo = mem[wa + 1];
                printf(" %02X%02X", hi, lo);
            } else if (wa < end) {
                uint8_t b = mem[wa];
                printf(" %02X", b);
                break;
            } else {
                break;
            }
        }
        printf("\n");
    }
}

