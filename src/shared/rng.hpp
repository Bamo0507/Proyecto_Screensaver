#pragma once

#include <cstdint>
#include <random>

// Mersenne Twister: misma semilla -> misma secuencia en cualquier máquina.
// Es lo que permite validar el kernel paralelo contra el secuencial.
using Rng = std::mt19937;

inline Rng seedGenerator(unsigned seed) {
    return Rng(seed);
}

// Decimal en [minimum, maximum). Para lo continuo: posiciones, velocidades,
// alpha, ángulos. Se normaliza a mano y no con uniform_real_distribution porque
// el estándar no garantiza que dos implementaciones den la misma secuencia.
inline float randomFloat(Rng& rng, float minimum, float maximum) {
    const float range = static_cast<float>(Rng::max() - Rng::min());
    const float t = static_cast<float>(rng() - Rng::min()) / (range + 1.0f);
    return minimum + t * (maximum - minimum);
}

// Entero en [minimum, maximum], ambos incluidos. Para lo discreto: duraciones
// en frames, cuál borde de la pantalla, cuántas partículas.
inline int randomInt(Rng& rng, int minimum, int maximum) {
    const uint32_t span = static_cast<uint32_t>(maximum - minimum + 1);
    return minimum + static_cast<int>(rng() % span);
}

// Índice válido para un arreglo de 'count' elementos: [0, count-1].
// Es el caso de la paleta: se sortea el índice del color, nunca el RGB.
inline int randomIndex(Rng& rng, int count) {
    return randomInt(rng, 0, count - 1);
}
