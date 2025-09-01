#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>   // rand
#include <time.h>     // time

#include "chip8.h"
#include "beep.h"
#include "timer.h"
#include <SDL3/SDL.h>

static void sdl_die(const char *where) {
    const char *e = SDL_GetError();
    fprintf(stderr, "[%s] SDL error: %s\n", where, (e && *e) ? e : "(empty)");
}

/* Map SDL keycode to CHIP-8 key index [0..15], return -1 if not a CHIP-8 key. */
static int map_sdl_key_to_chip8(SDL_Keycode kc) {
    switch (kc) {
        /* row 1: 1 2 3 4 -> 1 2 3 C(=12) */
        case SDLK_1: return 0x1;
        case SDLK_2: return 0x2;
        case SDLK_3: return 0x3;
        case SDLK_4: return 0xC;

        /* row 2: Q W E R -> 4 5 6 D(=13) */
        case SDLK_Q: return 0x4;
        case SDLK_W: return 0x5;
        case SDLK_E: return 0x6;
        case SDLK_R: return 0xD;

        /* row 3: A S D F -> 7 8 9 E(=14) */
        case SDLK_A: return 0x7;
        case SDLK_S: return 0x8;
        case SDLK_D: return 0x9;
        case SDLK_F: return 0xE;

        /* row 4: Z X C V -> A(=10) 0 B(=11) F(=15) */
        case SDLK_Z: return 0xA;
        case SDLK_X: return 0x0;
        case SDLK_C: return 0xB;
        case SDLK_V: return 0xF;
        default:     return -1;
    }
}

// Draw the logical screen buffer onto the SDL renderer.
static void draw_screen(SDL_Renderer* renderer, const Screen* scr) {
    const uint8_t* px = screen_pixels(scr);
    if (!px) return;

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    const int scale = EMULATOR_WINDOW_SCALER;

    for (int y = 0; y < DISPLAY_HEIGHT; ++y) {
        for (int x = 0; x < DISPLAY_WIDTH; ++x) {
            if (px[y * DISPLAY_WIDTH + x]) {
                SDL_FRect r = {
                    (float)(x * scale),
                    (float)(y * scale),
                    (float)scale,
                    (float)scale
                };
                SDL_RenderFillRect(renderer, &r);
            }
        }
    }

    SDL_RenderPresent(renderer);
}

int main(int argc, char** argv) {
    struct Chip8 chip8;
    chip8_init(&chip8);

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <path/to/game.ch8>\n", argv[0]);
        return 2;
    }
    Chip8Status st = chip8_load_rom(&chip8, argv[1]);
    if (st != CHIP8_OK) {
        fprintf(stderr, "Load failed: %s\n", chip8_status_str(st));
        return 1;
    }

    // mem_dump(&chip8, 0x200, 0x40);
    dump_n(&chip8, 0x200, 0x40, 8);
    return 0;
}