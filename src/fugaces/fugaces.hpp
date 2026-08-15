#pragma once

#include <SDL.h>

#include "shared/config.hpp"
#include "shared/rng.hpp"

// Capa 3 — estrellas fugaces que cruzan la pantalla. Solo secuencial.
namespace fugaces {

void init(const Config& cfg, Rng& rng);
void update(float dt);
void draw(SDL_Renderer* ren);
void destroy();

}  // namespace fugaces
