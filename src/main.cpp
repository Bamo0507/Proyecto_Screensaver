#include <SDL.h>
#include <cstdio>

#include "shared/config.hpp"

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    Config cfg;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::fprintf(stderr, "Error al inicializar SDL: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* ventana = SDL_CreateWindow("Screensaver - Constelacion",
                                           SDL_WINDOWPOS_CENTERED,
                                           SDL_WINDOWPOS_CENTERED,
                                           cfg.ancho, cfg.alto,
                                           SDL_WINDOW_SHOWN);
    if (!ventana) {
        std::fprintf(stderr, "Error al crear la ventana: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Con vsync mientras sea modo interactivo. En --bench se crea sin vsync
    SDL_Renderer* ren = SDL_CreateRenderer(ventana, -1,
                                           SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!ren) {
        std::fprintf(stderr, "Error al crear el renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(ventana);
        SDL_Quit();
        return 1;
    }

    bool corriendo = true;
    while (corriendo) {
        SDL_Event evento;
        while (SDL_PollEvent(&evento)) {
            if (evento.type == SDL_QUIT) {
                corriendo = false;
            } else if (evento.type == SDL_KEYDOWN && evento.key.keysym.sym == SDLK_ESCAPE) {
                corriendo = false;
            }
        }

        SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
        SDL_RenderClear(ren);

        // Aquí va el orden de dibujo de las 5 capas, de atrás hacia adelante, pintando cada una
        SDL_RenderPresent(ren);
    }

    // Se libera en orden inverso al de creación
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(ventana);
    SDL_Quit();
    return 0;
}
