#pragma once

#include <cstdint>
#include <random>

// Mersenne Twister: misma semilla -> misma secuencia en cualquier máquina.
// Es lo que permite validar el kernel paralelo contra el secuencial.
using Rng = std::mt19937;

inline Rng sembrarGenerador(unsigned semilla) {
    return Rng(semilla);
}

// Real en [minimo, maximo). Para lo continuo: posiciones, velocidades, alpha, ángulos.
// Se normaliza a mano y no con uniform_real_distribution porque el estándar no
// garantiza que dos implementaciones den la misma secuencia.
inline float realAleatorio(Rng& rng, float minimo, float maximo) {
    const float rango = static_cast<float>(Rng::max() - Rng::min());
    const float t = static_cast<float>(rng() - Rng::min()) / (rango + 1.0f);
    return minimo + t * (maximo - minimo);
}

// Entero en [minimo, maximo], ambos incluidos. Para lo discreto: duraciones en
// frames, cuál borde de la pantalla, cuántas partículas.
inline int enteroAleatorio(Rng& rng, int minimo, int maximo) {
    const uint32_t ancho = static_cast<uint32_t>(maximo - minimo + 1);
    return minimo + static_cast<int>(rng() % ancho);
}

// Índice válido para un arreglo de 'cantidad' elementos: [0, cantidad-1].
// Es el caso de la paleta: se sortea el índice del color, nunca el RGB.
inline int indiceAleatorio(Rng& rng, int cantidad) {
    return enteroAleatorio(rng, 0, cantidad - 1);
}
