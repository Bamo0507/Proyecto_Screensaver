#include "background/background.hpp"

#include <algorithm>
#include <cstdio>
#include <vector>

#include "shared/timing.hpp"

namespace {

std::vector<uint32_t> pixels;
SDL_Texture* texture = nullptr;

int textureWidth = 0;
int textureHeight = 0;
int windowWidth = 0;
int windowHeight = 0;

float fieldTime = 0.0f;
KernelParams kernelParams;
BackgroundKernel backgroundKernel = nullptr;

} // namespace

namespace background {

void init(const Config& cfg, Rng& rng) {
    (void)rng; // El campo es continuo y no requiere valores aleatorios.

    windowWidth = cfg.width;
    windowHeight = cfg.height;
    textureWidth = std::max(1, cfg.width / cfg.backgroundScale);
    textureHeight = std::max(1, cfg.height / cfg.backgroundScale);
    fieldTime = 0.0f;
    kernelParams = makeKernelParams(cfg);
    backgroundKernel = renderSequential;

    // El buffer se crea una sola vez. Reasignarlo cada frame agregaría ruido a
    // la medición del kernel y presión innecesaria al asignador de memoria.
    pixels.resize(static_cast<size_t>(textureWidth) * textureHeight);
}

void update(float dt) {
    fieldTime += dt;

    // Solo se mide el cálculo O(W*H); la transferencia SDL queda fuera porque
    // no es trabajo paralelizable y no debe contaminar t_fondo.
    timing::begin(timing::REGION_BACKGROUND);
    backgroundKernel(pixels.data(), textureWidth, textureHeight, fieldTime, kernelParams);
    timing::end(timing::REGION_BACKGROUND);
}

void draw(SDL_Renderer* renderer) {
    if (texture == nullptr) {
        texture = SDL_CreateTexture(renderer,
                                    SDL_PIXELFORMAT_ARGB8888,
                                    SDL_TEXTUREACCESS_STREAMING,
                                    textureWidth,
                                    textureHeight);
        if (texture == nullptr) {
            std::fprintf(stderr, "Error al crear la textura del fondo: %s\\n", SDL_GetError());
            return;
        }
    }

    const int pitch = textureWidth * static_cast<int>(sizeof(uint32_t));
    if (SDL_UpdateTexture(texture, nullptr, pixels.data(), pitch) != 0) {
        std::fprintf(stderr, "Error al actualizar la textura del fondo: %s\\n", SDL_GetError());
        return;
    }

    const SDL_Rect destination{0, 0, windowWidth, windowHeight};
    SDL_RenderCopy(renderer, texture, nullptr, &destination);
}

void destroy() {
    if (texture != nullptr) {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }

    pixels.clear();
    pixels.shrink_to_fit();
    textureWidth = 0;
    textureHeight = 0;
    windowWidth = 0;
    windowHeight = 0;
    fieldTime = 0.0f;
    backgroundKernel = nullptr;
}

} // namespace background
