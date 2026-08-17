#include <SDL.h>
#include <cstdio>

#include "constellation/constellation.hpp"
#include "shared/config.hpp"
#include "shared/rng.hpp"
#include "ship/ship.hpp"

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

    // Un solo generador para todas las capas: así la escena completa queda
    // determinada por --seed y se puede reproducir corrida tras corrida.
    Rng rng = seedGenerator(cfg.seed);
    constellation::init(cfg, rng);
    ship::init(cfg, rng);

    bool running = true;
    long framesDrawn = 0;

    const double counterFrequency = static_cast<double>(SDL_GetPerformanceFrequency());
    Uint64 previousCounter = SDL_GetPerformanceCounter();

    while (running) {
        const Uint64 currentCounter = SDL_GetPerformanceCounter();
        float dt = static_cast<float>((currentCounter - previousCounter) / counterFrequency);
        previousCounter = currentCounter;

        // Un frame que tardó demasiado (la ventana estuvo minimizada, el
        // sistema se trabó) daría un dt enorme y teletransportaría los nodos al
        // otro lado de la pantalla, saltándose el rebote. Se le pone techo.
        if (dt > 0.05f) {
            dt = 0.05f;
        }

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
                running = false;
            }
        }

        constellation::update(dt);
        ship::update(dt);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Orden de dibujo de las capas, de atrás hacia adelante.
        // Faltan background (0), stars (1) y meteors (3).
        constellation::draw(renderer); // capa 2
        ship::draw(renderer);          // capa 4

        SDL_RenderPresent(renderer);

        ++framesDrawn;
        if (cfg.benchFrames > 0 && framesDrawn >= cfg.benchFrames) {
            running = false;
        }
    }

    // Se libera en orden inverso al de creación.
    ship::destroy();
    constellation::destroy();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
