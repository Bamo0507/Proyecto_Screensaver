#pragma once

#include <cmath>
#include <cstdint>

struct Color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

// Paleta fija. Todo color del proyecto sale de aquí: los elementos sortean un
// índice sobre ella, nunca un RGB libre.
constexpr int PALETA_TAMANO = 7;

constexpr Color PALETA[PALETA_TAMANO] = {
    {3, 24, 35},     // Midnight Indigo       #031823
    {19, 44, 90},    // Deep Twilight Blue    #132C5A
    {36, 87, 152},   // Electric Sapphire     #245798
    {96, 82, 159},   // Dusky Periwinkle      #60529F
    {146, 62, 147},  // Orchid Glow           #923E93
    {146, 93, 177},  // Amethyst Haze         #925DB1
    {158, 115, 198}, // Soft Cosmic Lavender  #9E73C6
};

// Interpolación lineal entre dos colores. t = 0 devuelve a, t = 1 devuelve b.
inline Color mezclar(Color a, Color b, float t) {
    return Color{
        static_cast<uint8_t>(a.r + (b.r - a.r) * t),
        static_cast<uint8_t>(a.g + (b.g - a.g) * t),
        static_cast<uint8_t>(a.b + (b.b - a.b) * t)
    };
}

// Lleva cualquier valor, incluso negativo, al rango [0, periodo).
// fmod por sí solo conserva el signo: fmod(-0.5, 12) da -0.5, no 11.5.
inline float envolver(float valor, float periodo) {
    const float resto = std::fmod(valor, periodo);
    return resto < 0.0f ? resto + periodo : resto;
}

// Convierte un valor continuo (el campo del fondo más la fase global) en color.
//
// En planificación se acordó recorrer la paleta como anillo simple, uniendo
// PALETA[N-1] con PALETA[0]. Con la paleta que quedó elegida eso no funciona:
// va de casi negro a lavanda claro, así que al cerrar el círculo la luz cae de
// golpe y aparece un contorno oscuro recorriendo la nebulosa.
//
// Se optó entonces por un recorrido de ida y vuelta ("ping pong"): al llegar al
// tono más claro la secuencia se devuelve por el mismo camino en vez de saltar
// al más oscuro. Sigue siendo un anillo, solo que sobre la secuencia espejada,
// de período 2*N-2 = 12 en lugar de 7. La transición queda continua en ambos
// extremos y se elimina el salto raro entre el color más claro y el más oscuro.
inline Color desdePaleta(float valor) {
    constexpr float ULTIMO = PALETA_TAMANO - 1;       // 6
    constexpr float PERIODO = 2 * PALETA_TAMANO - 2;  // 12

    float p = envolver(valor, PERIODO);
    if (p > ULTIMO) {
        p = PERIODO - p; // el rebote: 7 se vuelve 5, 8 se vuelve 4, 11 se vuelve 1
    }

    const int indice = static_cast<int>(p);
    const int siguiente = (indice + 1 < PALETA_TAMANO) ? indice + 1 : indice;
    return mezclar(PALETA[indice], PALETA[siguiente], p - indice);
}

// El fondo se dibuja atenuado (~30%) para que no compita con la constelación.
inline Color atenuar(Color c, float factor) {
    return Color{
        static_cast<uint8_t>(c.r * factor),
        static_cast<uint8_t>(c.g * factor),
        static_cast<uint8_t>(c.b * factor)
    };
}

// Empaqueta a ARGB8888, que es el formato del buffer de píxeles del fondo.
inline uint32_t empacarPixel(Color c) {
    return (0xFFu << 24) |
           (static_cast<uint32_t>(c.r) << 16) |
           (static_cast<uint32_t>(c.g) << 8) |
           static_cast<uint32_t>(c.b);
}
