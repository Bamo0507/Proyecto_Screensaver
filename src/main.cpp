#include <SDL.h>
#include <cstdio>

#include "shared/config.hpp"

int main(int argc, char** argv) {
    Config cfg;
    const ArgsResult result = parseArguments(argc, argv, cfg);
    if (result == ArgsResult::Help) {
        return 0;
    }
    if (result == ArgsResult::Error) {
        return 1;
    }

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::fprintf(stderr, "Error al inicializar SDL: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Screensaver - Constelacion",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          cfg.width, cfg.height,
                                          SDL_WINDOW_SHOWN);
    if (!window) {
        std::fprintf(stderr, "Error al crear la ventana: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Con vsync mientras sea modo interactivo. En --bench se crea sin vsync,
    // porque con el refresco clavado a 60 FPS todo speedup mediria 1.00x.
    Uint32 rendererFlags = SDL_RENDERER_ACCELERATED;
    if (cfg.benchFrames == 0) {
        rendererFlags |= SDL_RENDERER_PRESENTVSYNC;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, rendererFlags);
    if (!renderer) {
        std::fprintf(stderr, "Error al crear el renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    bool running = true;
    long framesDrawn = 0;

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
                running = false;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Aquí va el orden de dibujo de las 5 capas, de atrás hacia adelante.

        SDL_RenderPresent(renderer);

        ++framesDrawn;
        if (cfg.benchFrames > 0 && framesDrawn >= cfg.benchFrames) {
            running = false;
        }
    }

    // Se libera en orden inverso al de creación.
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
