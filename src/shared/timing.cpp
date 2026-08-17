#include "shared/timing.hpp"

#include <chrono>
#include <cstdio>

#ifdef _OPENMP
#include <omp.h>
#endif

namespace {

using Clock = std::chrono::steady_clock;

// Peso del frame nuevo en la media móvil de FPS. Bajo para que el número del
// título se pueda leer, alto para que reaccione al subir N.
constexpr double FPS_WEIGHT = 0.1;

Config config;
bool csvEnabled = false;

Clock::time_point frameStart;
Clock::time_point regionStart[timing::REGION_COUNT];
double regionSeconds[timing::REGION_COUNT];

double frameSeconds = 0.0;
double fpsAverage = 0.0;
long frameIndex = 0;

double secondsSince(Clock::time_point start) {
    return std::chrono::duration<double>(Clock::now() - start).count();
}

// Cuántos hilos corre OpenMP de verdad. Con --hilos 0 el CSV no puede escribir
// "0": la fila quedaría sin el dato que da sentido al speedup.
int effectiveThreads(const Config& cfg) {
    if (!cfg.parallel) {
        return 1;
    }
#ifdef _OPENMP
    return (cfg.threads > 0) ? cfg.threads : omp_get_max_threads();
#else
    return (cfg.threads > 0) ? cfg.threads : 1;
#endif
}

} // namespace

namespace timing {

void init(const Config& cfg) {
    config = cfg;
    csvEnabled = (cfg.benchFrames > 0);
    frameIndex = 0;
    frameSeconds = 0.0;
    fpsAverage = 0.0;
    for (int i = 0; i < REGION_COUNT; ++i) {
        regionSeconds[i] = 0.0;
    }

    if (csvEnabled) {
        // Cada fila lleva la configuración completa para que varias corridas se
        // puedan concatenar en un solo archivo sin perder de cuál vino cada una.
        std::printf("n,radio,modo,hilos,secciones,trozo,frame,"
                    "t_fondo_ms,t_aristas_ms,t_render_ms,t_frame_ms,fps\n");
        std::fflush(stdout);
    }
}

void beginFrame() {
    for (int i = 0; i < REGION_COUNT; ++i) {
        regionSeconds[i] = 0.0;
    }
    frameStart = Clock::now();
}

void begin(Region region) {
    regionStart[region] = Clock::now();
}

void end(Region region) {
    regionSeconds[region] += secondsSince(regionStart[region]);
}

void endFrame() {
    frameSeconds = secondsSince(frameStart);

    const double instantFps = (frameSeconds > 0.0) ? (1.0 / frameSeconds) : 0.0;
    fpsAverage = (fpsAverage == 0.0)
                     ? instantFps
                     : fpsAverage * (1.0 - FPS_WEIGHT) + instantFps * FPS_WEIGHT;

    if (csvEnabled) {
        std::printf("%d,%.1f,%s,%d,%d,%d,%ld,%.4f,%.4f,%.4f,%.4f,%.2f\n",
                    config.nodeCount,
                    static_cast<double>(config.radius),
                    config.parallel ? "par" : "seq",
                    effectiveThreads(config),
                    config.sections,
                    config.chunk,
                    frameIndex,
                    regionSeconds[REGION_BACKGROUND] * 1000.0,
                    regionSeconds[REGION_EDGES] * 1000.0,
                    regionSeconds[REGION_RENDER] * 1000.0,
                    frameSeconds * 1000.0,
                    instantFps);
    }

    ++frameIndex;
}

double smoothedFps() {
    return fpsAverage;
}

double lastFrameSeconds() {
    return frameSeconds;
}

void destroy() {
    if (csvEnabled) {
        std::fflush(stdout);
    }
    csvEnabled = false;
    frameIndex = 0;
}

} // namespace timing
