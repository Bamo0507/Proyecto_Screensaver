#pragma once

#include <SDL.h>

#include "shared/config.hpp"
#include "shared/rng.hpp"

// Capa 0 — nebulosa animada de fondo. Región paralela B (O(W*H) pixeles).
namespace background {

// Kernel secuencial del campo. Se declara separado de la capa para que la
// futura variante OpenMP pueda intercambiarse sin cambiar el ciclo principal.
void renderSequential(uint32_t* target,
                      int width,
                      int height,
                      float time,
                      const KernelParams& params);

void init(const Config& cfg, Rng& rng);
void update(float dt);
void draw(SDL_Renderer* renderer);
void destroy();

} // namespace background
