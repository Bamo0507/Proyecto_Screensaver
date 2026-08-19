#include "stars/stars.hpp"

#include <algorithm>
#include <cmath>
#include <vector>

namespace {

// Una estrella por esta cantidad aproximada de píxeles mantiene la densidad
// visual al cambiar el tamaño de ventana, sin agregar otro parámetro de CLI.
constexpr int PIXELS_PER_STAR = 9000;
constexpr int MIN_STAR_COUNT = 24;

constexpr float TWINKLE_SPEED_MIN = 0.8f;
constexpr float TWINKLE_SPEED_MAX = 2.8f;
constexpr float TWO_PI = 6.28318530718f;
constexpr Uint8 CORE_ALPHA_MIN = 150;
constexpr Uint8 CORE_ALPHA_MAX = 255;
constexpr Uint8 HALO_ALPHA_MAX = 80;
constexpr Uint8 RAY_ALPHA_MAX = 115;
constexpr int CORE_SIZE = 2;
constexpr int HALO_SIZE = 4;

struct Star {
    float x;
    float y;
    float phase;
    float twinkleSpeed;
};

std::vector<Star> starList;
float elapsedTime = 0.0f;

} // namespace

namespace stars {

void init(const Config& cfg, Rng& rng) {
    elapsedTime = 0.0f;

    const int area = cfg.width * cfg.height;
    const int count = std::max(MIN_STAR_COUNT, area / PIXELS_PER_STAR);
    starList.resize(count);

    for (Star& star : starList) {
        star.x = randomFloat(rng, 0.0f, static_cast<float>(cfg.width));
        star.y = randomFloat(rng, 0.0f, static_cast<float>(cfg.height));
        star.phase = randomFloat(rng, 0.0f, TWO_PI);
        star.twinkleSpeed = randomFloat(rng, TWINKLE_SPEED_MIN, TWINKLE_SPEED_MAX);
    }
}

void update(float dt) {
    elapsedTime += dt;
}

void draw(SDL_Renderer* renderer) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    for (const Star& star : starList) {
        // La senoide desplaza el brillo de cada estrella por fase y velocidad,
        // evitando que toda la capa parpadee a la vez.
        const float brightness = 0.5f + 0.5f * std::sin(elapsedTime * star.twinkleSpeed + star.phase);
        const float coreRange = static_cast<float>(CORE_ALPHA_MAX - CORE_ALPHA_MIN);
        const Uint8 coreAlpha = static_cast<Uint8>(CORE_ALPHA_MIN + brightness * coreRange);
        const Uint8 haloAlpha = static_cast<Uint8>(brightness * brightness * HALO_ALPHA_MAX);
        const Uint8 rayAlpha = static_cast<Uint8>(brightness * brightness * RAY_ALPHA_MAX);
        const int rayLength = 1 + static_cast<int>(brightness * 3.0f);

        const int x = static_cast<int>(star.x);
        const int y = static_cast<int>(star.y);

        // El halo crece ópticamente al acercarse al brillo máximo. Se dibuja
        // antes del núcleo para que este conserve un blanco nítido.
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, haloAlpha);
        const SDL_Rect halo{x - 1, y - 1, HALO_SIZE, HALO_SIZE};
        SDL_RenderFillRect(renderer, &halo);

        // Rayos cortos en cruz: sugieren una estrella brillante sin convertir
        // el fondo en una capa de destellos que compita con la constelación.
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, rayAlpha);
        SDL_RenderDrawLine(renderer, x - rayLength, y, x + rayLength + 1, y);
        SDL_RenderDrawLine(renderer, x, y - rayLength, x, y + rayLength + 1);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, coreAlpha);
        const SDL_Rect core{x, y, CORE_SIZE, CORE_SIZE};
        SDL_RenderFillRect(renderer, &core);
    }
}

void destroy() {
    starList.clear();
    starList.shrink_to_fit();
    elapsedTime = 0.0f;
}

} // namespace stars
