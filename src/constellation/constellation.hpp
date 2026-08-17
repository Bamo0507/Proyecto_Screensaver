#pragma once

#include <SDL.h>
#include <vector>

#include "shared/config.hpp"
#include "shared/kernel.hpp"
#include "shared/rng.hpp"

// Una arista detectada en el frame actual.
//
// Guarda índices y no punteros ni coordenadas: los índices sobreviven a que los
// arreglos se reubiquen en memoria, y con ellos se llega tanto a la posición
// como al color de cada extremo.
//
// closeness va de 0 (justo en el borde del radio) a 1 (nodos encima uno de
// otro). Se calcula durante la detección, que ya tiene la distancia en la mano,
// para que draw no tenga que volver a computarla.
struct Edge {
    int nodeA;
    int nodeB;
    float closeness;
};

// Kernels de detección. Comparten firma exacta para poder intercambiarse en
// runtime a través de un EdgeKernel.
void detectEdgesSequential(const float* posX,
                           const float* posY,
                           int count,
                           float radiusSquared,
                           std::vector<Edge>& out,
                           const KernelParams& params);

// Definido en edges_par.cpp a partir del escalón v2.
void detectEdgesParallel(const float* posX,
                         const float* posY,
                         int count,
                         float radiusSquared,
                         std::vector<Edge>& out,
                         const KernelParams& params);

// Capa 2 — nodos que rebotan y aristas entre los cercanos.
// Región paralela A (O(N^2) pares).
namespace constellation {

void init(const Config& cfg, Rng& rng);
void update(float dt);
void draw(SDL_Renderer* renderer);
void destroy();

} // namespace constellation
