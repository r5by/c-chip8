#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>   // rand
#include <time.h>     // time

#include "keyboard.h"
#include "chip8_status.h"
#include "config.h"
#include "chip8.h"
#include "screen.h"

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

int main(int argc, char **argv) {
    struct Chip8 chip8;
    chip8_init(&chip8);

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        sdl_die("SDL_Init(SDL_INIT_VIDEO)");
        return 1;
    }

    const int win_w = DISPLAY_WIDTH  * EMULATOR_WINDOW_SCALER;
    const int win_h = DISPLAY_HEIGHT * EMULATOR_WINDOW_SCALER;

    SDL_Window *window = SDL_CreateWindow(
        "Chip8 Window",
        win_w, win_h,
        SDL_WINDOW_RESIZABLE
    );
    if (!window) { sdl_die("SDL_CreateWindow"); SDL_Quit(); return 1; }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) { sdl_die("SDL_CreateRenderer"); SDL_DestroyWindow(window); SDL_Quit(); return 1; }

    // Demo: draw glyphs 0..F at (32,32), switching every 1s (wrap-around expected)
    const uint8_t POS_X = 33;
    const uint8_t POS_Y = 61;            // 32 == DISPLAY_HEIGHT -> wraps to 0
    uint8_t glyph = 0;                   // 0..15

    Uint64 last_switch_ns = SDL_GetTicksNS();
    bool running = true;

    while (running) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_EVENT_QUIT) {
                running = false;
            } else if (ev.type == SDL_EVENT_KEY_DOWN) {
                SDL_Keycode kc = ev.key.key;          /* SDL3: 'key' holds SDL_Keycode */
                int ck = map_sdl_key_to_chip8(kc);
                if (ck >= 0) {
                    Chip8Status st = keyboard_press(&chip8.chip8_kbd, (uint8_t)ck);
                    if (st == CHIP8_OK) {
                        printf("[key down] chip8=0x%X\n", ck);
                    } else {
                        CHIP8_LOG_ERROR("keyboard_press failed: %s (chip8=0x%X)",
                                        chip8_status_str(st), ck);
                    }
                } else {
                    CHIP8_LOG_WARN("unknown key pressed (SDL_Keycode=%d)", (int)kc);
                }
            } else if (ev.type == SDL_EVENT_KEY_UP) {
                SDL_Keycode kc = ev.key.key;
                int ck = map_sdl_key_to_chip8(kc);
                if (ck >= 0) {
                    Chip8Status st = keyboard_release(&chip8.chip8_kbd, (uint8_t)ck);
                    if (st == CHIP8_OK) {
                        printf("[key up  ] chip8=0x%X\n", ck);
                    } else {
                        CHIP8_LOG_ERROR("keyboard_release failed: %s (chip8=0x%X)",
                                        chip8_status_str(st), ck);
                    }
                } else {
                    CHIP8_LOG_WARN("unknown key released (SDL_Keycode=%d)", (int)kc);
                }
            }
        }

        // Switch glyph every 1 second
        Uint64 now_ns = SDL_GetTicksNS();
        if (now_ns - last_switch_ns >= 1000000000ULL) { // 1e9 ns
            last_switch_ns = now_ns;

            // Clear screen then draw next glyph from RAM fontset
            screen_clear(&chip8.chip8_disp);

            // Fontset layout: 16 glyphs × 5 bytes each at FONT_START_ADDR
            size_t offset = (size_t)FONT_START_ADDR + (size_t)glyph * 5u;
            const uint8_t* sprite = &chip8.chip8_mem.memory[offset];   // uses initialized RAM fontset
            (void)screen_draw_sprite(&chip8.chip8_disp, POS_X, POS_Y, sprite, 5);

            glyph = (uint8_t)((glyph + 1) & 0x0F); // 0..15
        }

        if (screen_consume_dirty(&chip8.chip8_disp)) {
            draw_screen(renderer, &chip8.chip8_disp);
        }

        SDL_Delay(16); // ~60 FPS
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}