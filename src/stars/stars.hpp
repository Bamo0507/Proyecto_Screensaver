#pragma once

#include <SDL.h>

#include "shared/config.hpp"
#include "shared/rng.hpp"

// Capa 1 — estrellas de fondo con parpadeo. Solo secuencial.
namespace stars {

void init(const Config& cfg, Rng& rng);
void update(float dt);
void draw(SDL_Renderer* renderer);
void destroy();

} // namespace stars
