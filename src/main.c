// src/main.c
#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "config.h"
#include "chip8.h"
#include "chip8_status.h"
#include "mem.h"
#include "screen.h"
#include "keyboard.h"
#include "instr.h"
#include "timer.h"
#include "beep.h"   // Beeper*, bool beep_init(Beeper** , int freq_hz, float volume); void beep_set(Beeper*, bool on);

/* Map SDL keycode to CHIP-8 key index [0..15], return -1 if not a CHIP-8 key. */
static int map_sdl_key_to_chip8(SDL_Keycode kc) {
    switch (kc) {
        /* row 1: 1 2 3 4 -> 1 2 3 C */
        case SDLK_1: return 0x1;
        case SDLK_2: return 0x2;
        case SDLK_3: return 0x3;
        case SDLK_4: return 0xC;

        /* row 2: Q W E R -> 4 5 6 D */
        case SDLK_Q: return 0x4;
        case SDLK_W: return 0x5;
        case SDLK_E: return 0x6;
        case SDLK_R: return 0xD;

        /* row 3: A S D F -> 7 8 9 E */
        case SDLK_A: return 0x7;
        case SDLK_S: return 0x8;
        case SDLK_D: return 0x9;
        case SDLK_F: return 0xE;

        /* row 4: Z X C V -> A 0 B F */
        case SDLK_Z: return 0xA;
        case SDLK_X: return 0x0;
        case SDLK_C: return 0xB;
        case SDLK_V: return 0xF;
        default:     return -1;
    }
}

/* Minimal SDL error logger. */
static void sdl_die(const char *where) {
    const char *e = SDL_GetError();
    fprintf(stderr, "[%s] SDL error: %s\n", where, (e && *e) ? e : "(empty)");
}

/* Render the logical 64x32 display buffer to the SDL renderer. */
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
                SDL_FRect r = { (float)(x * scale), (float)(y * scale),
                                (float)scale,        (float)scale };
                SDL_RenderFillRect(renderer, &r);
            }
        }
    }
    SDL_RenderPresent(renderer);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <path/to/rom>\n", (argc > 0 ? argv[0] : "chip8"));
        return 2;
    }
    const char* rom_path = argv[1];

    struct Chip8 chip8;
    chip8_init(&chip8);

    /* Load ROM into memory at PROGRAM_START_ADDRESS (0x200). */
    Chip8Status rst = chip8_load_rom(&chip8, rom_path);
    if (rst != CHIP8_OK) {
        fprintf(stderr, "Failed to load ROM: %s (%s)\n", rom_path, chip8_status_str(rst));
        return 3;
    }
    chip8.chip8_regs.PC = PROGRAM_START_ADDRESS;

    /* Init SDL (video + audio). */
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        sdl_die("SDL_Init(VIDEO|AUDIO)");
        return 1;
    }

    /* Initialize beeper: ~330 Hz, gentle volume (0.15). */
    Beeper* beeper = NULL;
    if (!beep_init(&beeper, 330, 0.15f)) {
        SDL_LogWarn(SDL_LOG_CATEGORY_AUDIO, "beep init failed: %s", SDL_GetError());
        beeper = NULL; // continue without sound
    }

    /* Create window and renderer. */
    const int win_w = DISPLAY_WIDTH  * EMULATOR_WINDOW_SCALER;
    const int win_h = DISPLAY_HEIGHT * EMULATOR_WINDOW_SCALER;

    SDL_Window *window = SDL_CreateWindow("Chip8 Window", win_w, win_h, SDL_WINDOW_RESIZABLE);
    if (!window) { sdl_die("SDL_CreateWindow"); SDL_Quit(); return 1; }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) { sdl_die("SDL_CreateRenderer"); SDL_DestroyWindow(window); SDL_Quit(); return 1; }

    /* Timing configuration:
       - CPU runs ~700 Hz (typical range 500..1000)
       - DT/ST are updated at 60 Hz inside regs_tick_timers() using wall-clock ns
     */
    const uint64_t NS_PER_SEC   = 1000000000ull;
    const uint64_t CPU_HZ       = 700ull;
    const uint64_t NS_PER_CYCLE = NS_PER_SEC / CPU_HZ;

    uint64_t last_ns  = SDL_GetTicksNS();
    uint64_t accum_ns = 0;

    bool running = true;
    while (running) {
        /* Process SDL events and feed our CHIP-8 keyboard state. */
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_EVENT_QUIT) {
                running = false;
            } else if (ev.type == SDL_EVENT_KEY_DOWN) {
                SDL_Keycode kc = ev.key.key;  // SDL3 stores SDL_Keycode in ev.key.key
                int ck = map_sdl_key_to_chip8(kc);
                if (ck >= 0) {
                    Chip8Status st = keyboard_press(&chip8.chip8_kbd, (uint8_t)ck);
                    if (st != CHIP8_OK) {
                        CHIP8_LOG_ERROR("keyboard_press failed: %s (chip8=0x%X)",
                                        chip8_status_str(st), ck);
                    }
                }
            } else if (ev.type == SDL_EVENT_KEY_UP) {
                SDL_Keycode kc = ev.key.key;
                int ck = map_sdl_key_to_chip8(kc);
                if (ck >= 0) {
                    Chip8Status st = keyboard_release(&chip8.chip8_kbd, (uint8_t)ck);
                    if (st != CHIP8_OK) {
                        CHIP8_LOG_ERROR("keyboard_release failed: %s (chip8=0x%X)",
                                        chip8_status_str(st), ck);
                    }
                }
            }
        }

        /* Accumulate wall-clock time and run as many CPU cycles as fit the budget. */
        const uint64_t now_ns = SDL_GetTicksNS();
        const uint64_t dt_ns  = now_ns - last_ns;
        last_ns = now_ns;
        accum_ns += dt_ns;

        while (accum_ns >= NS_PER_CYCLE) {
            accum_ns -= NS_PER_CYCLE;

            /* One CPU step: fetch+execute. chip8_step() does PC += 2 pre-step internally. */
            Chip8Status cs = chip8_step(&chip8);
            if (cs != CHIP8_OK) {
                CHIP8_LOG_ERROR("chip8_step failed: %s (PC=0x%03X)",
                                chip8_status_str(cs), chip8.chip8_regs.PC);
                running = false;
                break;
            }
        }

        /* 60 Hz timers (DT/ST) and beep control. */
        bool start_beep = false, stop_beep = false;
        regs_tick_timers(&chip8.chip8_regs, dt_ns, &start_beep, &stop_beep);
        if (start_beep && beeper) beep_set(beeper, true);
        if (stop_beep  && beeper) beep_set(beeper, false);

        /* Redraw only when display changed. */
        if (screen_consume_dirty(&chip8.chip8_disp)) {
            draw_screen(renderer, &chip8.chip8_disp);
        }

        /* Small sleep to keep CPU usage in check. */
        SDL_Delay(1);
    }

    /* Stop beep (if any) before shutdown. */
    if (beeper) beep_set(beeper, false);
    beep_destroy(beeper);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
