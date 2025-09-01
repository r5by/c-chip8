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

int main(int argc, char **argv) {
    struct Chip8 chip8;
    chip8_init(&chip8);

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        sdl_die("SDL_Init(SDL_INIT_VIDEO)");
        return 1;
    }

    const int win_w = DISPLAY_WIDTH  * EMULATOR_WINDOW_SCALER;
    const int win_h = DISPLAY_HEIGHT * EMULATOR_WINDOW_SCALER;

    Beeper* beeper = NULL;
    if (!beep_init(&beeper, 330, 0.15f)) {
        SDL_LogWarn(SDL_LOG_CATEGORY_AUDIO, "beep init failed: %s", SDL_GetError());
    }

    // 3) Set up registers and initialize timers
    Registers regs = {0};
    // DT: 2 seconds（~120 60Hz ticks），ST: 0.5 second（~ 30 ticks）
    regs.DT = 120;
    regs.ST = 30;
    SDL_Log("Initial DT=%u, ST=%u", regs.DT, regs.ST);

    SDL_Window *window = SDL_CreateWindow(
        "Chip8 Window",
        win_w, win_h,
        SDL_WINDOW_RESIZABLE
    );
    if (!window) { sdl_die("SDL_CreateWindow"); SDL_Quit(); return 1; }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) { sdl_die("SDL_CreateRenderer"); SDL_DestroyWindow(window); SDL_Quit(); return 1; }

    Uint64 last_switch_ns = SDL_GetTicksNS();
    bool running = true;

    uint8_t prev_DT = regs.DT, prev_ST = regs.ST;
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

        // frame pass time (ns)
        Uint64 now_ns = SDL_GetTicksNS();
        Uint64 elapsed_ns = now_ns - last_switch_ns;
        last_switch_ns = now_ns;

        // step 60Hz timer；decides if open/close beep based on ST
        bool start_beep = false, stop_beep = false;
        regs_tick_timers(&regs, elapsed_ns, &start_beep, &stop_beep);

        if (start_beep && beeper) beep_set(beeper, true);
        if (stop_beep  && beeper) beep_set(beeper, false);

        // print change of DT/ST
        if (regs.DT != prev_DT || regs.ST != prev_ST) {
            SDL_Log("DT=%u, ST=%u", regs.DT, regs.ST);
            prev_DT = regs.DT;
            prev_ST = regs.ST;
        }

        if (regs.DT == 0 && regs.ST == 0) {
            running = false;
        }

        SDL_Delay(16); // ~60 FPS
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}