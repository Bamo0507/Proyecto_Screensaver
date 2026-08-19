#include "meteors/meteors.hpp"

#include <cmath>
#include <vector>

#include "shared/palette.hpp"

namespace {

constexpr int METEOR_COUNT = 4;
constexpr float WAIT_MIN = 1.2f;
constexpr float WAIT_MAX = 4.5f;
constexpr float SPEED_X_MIN = 280.0f;
constexpr float SPEED_X_MAX = 480.0f;
constexpr float SPEED_Y_MIN = 130.0f;
constexpr float SPEED_Y_MAX = 330.0f;
constexpr float TAIL_LENGTH = 42.0f;
constexpr int TRAIL_SEGMENTS = 3;
constexpr int HEAD_SIZE = 3;
constexpr int GLOW_SIZE = 7;

// Excepción visual solicitada para distinguir las fugaces de las demás capas,
// que continúan usando exclusivamente la paleta violeta del proyecto.
constexpr Color METEOR_COLOR{255, 240, 170};

struct Meteor {
    float x;
    float y;
    float velocityX;
    float velocityY;
    float wait;
    Color color;
    bool active;
};

std::vector<Meteor> meteorList;
Config config;
Rng* generator = nullptr;

// La fuga entra desde arriba o desde la izquierda y siempre cruza la escena
// en diagonal hacia abajo y a la derecha. El margen evita que aparezca ya
// visible en el borde.
void spawn(Meteor& meteor) {
    const bool fromLeft = randomInt(*generator, 0, 1) == 0;
    if (fromLeft) {
        meteor.x = -TAIL_LENGTH;
        meteor.y = randomFloat(*generator,
                               -static_cast<float>(config.height) * 0.15f,
                               static_cast<float>(config.height) * 0.65f);
    } else {
        meteor.x = randomFloat(*generator,
                               -static_cast<float>(config.width) * 0.15f,
                               static_cast<float>(config.width) * 0.70f);
        meteor.y = -TAIL_LENGTH;
    }

    meteor.velocityX = randomFloat(*generator, SPEED_X_MIN, SPEED_X_MAX);
    meteor.velocityY = randomFloat(*generator, SPEED_Y_MIN, SPEED_Y_MAX);
    meteor.color = METEOR_COLOR;
    meteor.active = true;
}

void scheduleNext(Meteor& meteor) {
    meteor.wait = randomFloat(*generator, WAIT_MIN, WAIT_MAX);
    meteor.active = false;
}

} // namespace

namespace meteors {

void init(const Config& cfg, Rng& rng) {
    config = cfg;
    generator = &rng;
    meteorList.resize(METEOR_COUNT);

    for (Meteor& meteor : meteorList) {
        // Se escalonan los primeros nacimientos: si todos arrancaran en cero,
        // las fugaces se moverían como un grupo y perderían naturalidad.
        meteor.wait = randomFloat(rng, 0.0f, WAIT_MAX);
        meteor.active = false;
        meteor.color = METEOR_COLOR;
    }
}

void update(float dt) {
    for (Meteor& meteor : meteorList) {
        if (!meteor.active) {
            meteor.wait -= dt;
            if (meteor.wait <= 0.0f) {
                spawn(meteor);
            }
            continue;
        }

        meteor.x += meteor.velocityX * dt;
        meteor.y += meteor.velocityY * dt;

        if (meteor.x > static_cast<float>(config.width) + TAIL_LENGTH ||
            meteor.y > static_cast<float>(config.height) + TAIL_LENGTH) {
            scheduleNext(meteor);
        }
    }
}

void draw(SDL_Renderer* renderer) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    for (const Meteor& meteor : meteorList) {
        if (!meteor.active) {
            continue;
        }

        const Color color = meteor.color;
        const float speed = std::sqrt(meteor.velocityX * meteor.velocityX +
                                      meteor.velocityY * meteor.velocityY);
        const float directionX = meteor.velocityX / speed;
        const float directionY = meteor.velocityY / speed;

        // La estela se parte para simular que se desvanece. SDL no dibuja
        // gradientes de línea, así que cada segmento usa menos alpha.
        for (int segment = 0; segment < TRAIL_SEGMENTS; ++segment) {
            const float from = TAIL_LENGTH * segment / TRAIL_SEGMENTS;
            const float to = TAIL_LENGTH * (segment + 1) / TRAIL_SEGMENTS;
            const Uint8 alpha = static_cast<Uint8>(150 - segment * 40);
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, alpha);
            SDL_RenderDrawLine(renderer,
                               static_cast<int>(meteor.x - directionX * from),
                               static_cast<int>(meteor.y - directionY * from),
                               static_cast<int>(meteor.x - directionX * to),
                               static_cast<int>(meteor.y - directionY * to));
        }

        // Capas cuadradas translúcidas alrededor del núcleo: simulan brillo
        // sin texturas ni llamadas extra a SDL fuera del hilo maestro.
        const SDL_Rect glow{static_cast<int>(meteor.x) - GLOW_SIZE / 2,
                            static_cast<int>(meteor.y) - GLOW_SIZE / 2,
                            GLOW_SIZE,
                            GLOW_SIZE};
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 38);
        SDL_RenderFillRect(renderer, &glow);

        const Color hotColor = blend(color, Color{255, 255, 255}, 0.55f);
        const SDL_Rect innerGlow{static_cast<int>(meteor.x) - 2,
                                 static_cast<int>(meteor.y) - 2,
                                 5,
                                 5};
        SDL_SetRenderDrawColor(renderer, hotColor.r, hotColor.g, hotColor.b, 90);
        SDL_RenderFillRect(renderer, &innerGlow);

        SDL_SetRenderDrawColor(renderer, hotColor.r, hotColor.g, hotColor.b, 255);
        const SDL_Rect head{static_cast<int>(meteor.x) - HEAD_SIZE / 2,
                            static_cast<int>(meteor.y) - HEAD_SIZE / 2,
                            HEAD_SIZE,
                            HEAD_SIZE};
        SDL_RenderFillRect(renderer, &head);
    }
}

void destroy() {
    meteorList.clear();
    meteorList.shrink_to_fit();
    generator = nullptr;
}

} // namespace meteors
