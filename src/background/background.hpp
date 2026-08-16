#pragma once

#include <SDL.h>

#include "shared/config.hpp"
#include "shared/rng.hpp"

// Capa 0 — nebulosa animada de fondo. Región paralela B (O(W*H) pixeles).
namespace background {

void init(const Config& cfg, Rng& rng);
void update(float dt);
void draw(SDL_Renderer* renderer);
void destroy();

} // namespace background
