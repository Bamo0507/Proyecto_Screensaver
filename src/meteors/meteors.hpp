#pragma once

#include <SDL.h>

#include "shared/config.hpp"
#include "shared/rng.hpp"

// Capa 3 — estrellas fugaces que cruzan la pantalla. Solo secuencial.
namespace meteors {

void init(const Config& cfg, Rng& rng);
void update(float dt);
void draw(SDL_Renderer* renderer);
void destroy();

} // namespace meteors
