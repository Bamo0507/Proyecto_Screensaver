#pragma once

#include <SDL.h>

#include "shared/config.hpp"
#include "shared/rng.hpp"

// Capa 2 — nodos que rebotan y aristas entre los cercanos.
// Región paralela A (O(N^2) pares).
namespace constellation {

void init(const Config& cfg, Rng& rng);
void update(float dt);
void draw(SDL_Renderer* renderer);
void destroy();

} // namespace constellation
