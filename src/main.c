#include <SDL3/SDL.h>
#include <stdio.h>

static void sdl_die(const char *where) {
    const char *e = SDL_GetError();
    fprintf(stderr, "[%s] SDL error: %s\n", where, (e && *e) ? e : "(empty)");
}

int main(int argc, char **argv) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        sdl_die("SDL_Init(SDL_INIT_VIDEO)");
        return 1;
    }

    // SDL3: 只有 (title, w, h, flags)
    SDL_Window *window = SDL_CreateWindow(
        "Chip8 Window",
        640, 320,
        SDL_WINDOW_RESIZABLE                // 或 SDL_WINDOW_RESIZABLE
    );
    if (!window) { sdl_die("SDL_CreateWindow"); SDL_Quit(); return 1; }

    // SDL3: 第二个参数 renderer 名称，NULL 让 SDL 选择默认渲染器
    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) { sdl_die("SDL_CreateRenderer"); SDL_DestroyWindow(window); SDL_Quit(); return 1; }

    // 简单主循环：清屏→画矩形描边→显示；按关闭按钮退出
    bool running = true;
    while (running) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_EVENT_QUIT) running = false;
        }

        // 背景：黑
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // 前景：白，画一个 40x40 的描边矩形（左上角 0,0）
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        const int x = 0, y = 0, w = 40, h = 40;
        // 用 4 条线画出矩形边框（避免 SDL2/SDL3 对 Fill/DrawRect 命名差异）
        SDL_RenderLine(renderer, x,     y,     x + w - 1, y);
        SDL_RenderLine(renderer, x,     y,     x,         y + h - 1);
        SDL_RenderLine(renderer, x+w-1, y,     x + w - 1, y + h - 1);
        SDL_RenderLine(renderer, x,     y+h-1, x + w - 1, y + h - 1);

        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ~60 FPS
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
