#include "constellation/constellation.hpp"

// Detección secuencial de aristas: el baseline contra el que se mide todo.
//
// Este archivo NO lleva un solo #pragma, ni siquiera uno con num_threads(1).
// Un kernel paralelo forzado a un hilo seguiría pagando fork-join en cada uno
// de los ~60 frames por segundo, inflaría T1 y con eso todos los speedups
// saldrían mentirosamente altos.
void detectEdgesSequential(const float* posX,
                           const float* posY,
                           int count,
                           float radiusSquared,
                           std::vector<Edge>& out,
                           const KernelParams& params) {
    (void)params; // el secuencial no usa hilos, secciones ni bloques

    out.clear(); // conserva la capacidad ya reservada, no libera memoria

    for (int i = 0; i < count; ++i) {
        const float xi = posX[i];
        const float yi = posY[i];

        // Arranca en i+1 y no en 0: así cada par se visita una sola vez. Si
        // recorriera todos contra todos saldría cada arista duplicada y se
        // dibujaría dos veces encima. Esto es lo que hace el bucle triangular
        // (y, de paso, lo que lo vuelve desbalanceado al paralelizarlo).
        for (int j = i + 1; j < count; ++j) {
            const float dx = xi - posX[j];
            const float dy = yi - posY[j];
            const float distanceSquared = dx * dx + dy * dy;

            // Se compara contra el radio al cuadrado para no llamar a sqrt
            // dentro del bucle más caliente del programa.
            if (distanceSquared < radiusSquared) {
                Edge edge;
                edge.nodeA = i;
                edge.nodeB = j;
                edge.closeness = 1.0f - distanceSquared / radiusSquared;
                out.push_back(edge);
            }
        }
    }
}
