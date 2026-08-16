#pragma once

#include <SDL.h>

#include "shared/config.hpp"
#include "shared/rng.hpp"

// Capa 4 — nave que cruza y reaparece cambiando de color. Solo secuencial.
namespace ship {

void init(const Config& cfg, Rng& rng);
void update(float dt);
void draw(SDL_Renderer* renderer);
void destroy();

} // namespace ship
