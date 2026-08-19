#include <SDL.h>
#include <cstdio>

#include "background/background.hpp"
#include "constellation/constellation.hpp"
#include "shared/config.hpp"
#include "shared/rng.hpp"
#include "shared/timing.hpp"
#include "ship/ship.hpp"

namespace {

// SDL_SetWindowTitle habla con el gestor de ventanas: refrescarlo por frame
// cuesta más que dibujar la escena.
constexpr double TITLE_INTERVAL = 0.5;

// Techo del dt para que un frame trabado no teletransporte los nodos.
constexpr float MAX_DT = 0.05f;

void buildTitle(char* out, size_t size, const Config& cfg, double fps) {
    char threads[32];
    if (!cfg.parallel) {
        std::snprintf(threads, sizeof(threads), "-");
    } else if (cfg.threads == 0) {
        std::snprintf(threads, sizeof(threads), "auto");
    } else {
        std::snprintf(threads, sizeof(threads), "%d", cfg.threads);
    }

    std::snprintf(out, size,
                  "Constelacion  |  N=%d  radio=%.0f  |  %s  hilos=%s  |  %.1f FPS",
                  cfg.nodeCount,
                  static_cast<double>(cfg.radius),
                  cfg.parallel ? "paralelo" : "secuencial",
                  threads,
                  fps);
}

} // namespace

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

    SDL_Window* window = SDL_CreateWindow("Constelacion",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          cfg.width, cfg.height,
                                          SDL_WINDOW_SHOWN);
    if (!window) {
        std::fprintf(stderr, "Error al crear la ventana: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // En --bench se crea sin vsync: con el refresco clavado a 60 FPS todo
    // speedup mediria 1.00x.
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

    // Un solo generador para todas las capas: reordenar estas llamadas cambia
    // la escena aunque la semilla sea la misma.
    Rng rng = seedGenerator(cfg.seed);
    background::init(cfg, rng);
    // TODO: stars::init, meteors::init
    constellation::init(cfg, rng);
    ship::init(cfg, rng);
    timing::init(cfg);

    bool running = true;
    long framesDrawn = 0;
    double titleElapsed = 0.0;

    const double counterFrequency = static_cast<double>(SDL_GetPerformanceFrequency());
    Uint64 previousCounter = SDL_GetPerformanceCounter();

    while (running) {
        timing::beginFrame();

        const Uint64 currentCounter = SDL_GetPerformanceCounter();
        float dt = static_cast<float>((currentCounter - previousCounter) / counterFrequency);
        previousCounter = currentCounter;
        if (dt > MAX_DT) {
            dt = MAX_DT;
        }

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
                running = false;
            }
        }

        background::update(dt);
        // TODO: stars::update, meteors::update
        constellation::update(dt);
        ship::update(dt);

        timing::begin(timing::REGION_RENDER);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // El z-order es el orden de estas lineas, de atras hacia adelante.
        background::draw(renderer); // capa 0
        // TODO: stars::draw (capa 1)
        constellation::draw(renderer); // capa 2
        // TODO: meteors::draw (capa 3)
        ship::draw(renderer); // capa 4

        SDL_RenderPresent(renderer);
        timing::end(timing::REGION_RENDER);

        timing::endFrame();
        ++framesDrawn;

        titleElapsed += dt;
        if (titleElapsed >= TITLE_INTERVAL) {
            char title[160];
            buildTitle(title, sizeof(title), cfg, timing::smoothedFps());
            SDL_SetWindowTitle(window, title);
            titleElapsed = 0.0;
        }

        if (cfg.benchFrames > 0 && framesDrawn >= cfg.benchFrames) {
            running = false;
        }
    }

    // Las capas van antes que el renderer: sus texturas se crearon a partir de
    // el. Despues, orden inverso al de creacion.
    // TODO: meteors::destroy, stars::destroy
    timing::destroy();
    ship::destroy();
    constellation::destroy();
    background::destroy();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
