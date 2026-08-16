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
constexpr int PALETTE_SIZE = 7;

constexpr Color PALETTE[PALETTE_SIZE] = {
    {3, 24, 35},     // Midnight Indigo       #031823
    {19, 44, 90},    // Deep Twilight Blue    #132C5A
    {36, 87, 152},   // Electric Sapphire     #245798
    {96, 82, 159},   // Dusky Periwinkle      #60529F
    {146, 62, 147},  // Orchid Glow           #923E93
    {146, 93, 177},  // Amethyst Haze         #925DB1
    {158, 115, 198}, // Soft Cosmic Lavender  #9E73C6
};

// Interpolación lineal entre dos colores. t = 0 devuelve a, t = 1 devuelve b.
inline Color blend(Color a, Color b, float t) {
    return Color{
        static_cast<uint8_t>(a.r + (b.r - a.r) * t),
        static_cast<uint8_t>(a.g + (b.g - a.g) * t),
        static_cast<uint8_t>(a.b + (b.b - a.b) * t)
    };
}

// Lleva cualquier valor, incluso negativo, al rango [0, period).
// fmod por sí solo conserva el signo: fmod(-0.5, 12) da -0.5, no 11.5.
inline float wrap(float value, float period) {
    const float remainder = std::fmod(value, period);
    return remainder < 0.0f ? remainder + period : remainder;
}

// Convierte un valor continuo (el campo del fondo más la fase global) en color.
//
// En planificación se acordó recorrer la paleta como anillo simple, uniendo
// PALETTE[N-1] con PALETTE[0]. Con la paleta que quedó elegida eso no funciona:
// va de casi negro a lavanda claro, así que al cerrar el círculo la luz cae de
// golpe y aparece un contorno oscuro recorriendo la nebulosa.
//
// Se optó entonces por un recorrido de ida y vuelta ("ping pong"): al llegar al
// tono más claro la secuencia se devuelve por el mismo camino en vez de saltar
// al más oscuro. Sigue siendo un anillo, solo que sobre la secuencia espejada,
// de período 2*N-2 = 12 en lugar de 7. La transición queda continua en ambos
// extremos y se elimina el salto raro entre el color más claro y el más oscuro.
inline Color fromPalette(float value) {
    constexpr float LAST = PALETTE_SIZE - 1;      // 6
    constexpr float PERIOD = 2 * PALETTE_SIZE - 2; // 12

    float p = wrap(value, PERIOD);
    if (p > LAST) {
        p = PERIOD - p; // el rebote: 7 se vuelve 5, 8 se vuelve 4, 11 se vuelve 1
    }

    const int index = static_cast<int>(p);
    const int next = (index + 1 < PALETTE_SIZE) ? index + 1 : index;
    return blend(PALETTE[index], PALETTE[next], p - index);
}

// El fondo se dibuja atenuado (~30%) para que no compita con la constelación.
inline Color dim(Color c, float factor) {
    return Color{
        static_cast<uint8_t>(c.r * factor),
        static_cast<uint8_t>(c.g * factor),
        static_cast<uint8_t>(c.b * factor)
    };
}

// Empaqueta a ARGB8888, que es el formato del buffer de píxeles del fondo.
inline uint32_t packPixel(Color c) {
    return (0xFFu << 24) |
           (static_cast<uint32_t>(c.r) << 16) |
           (static_cast<uint32_t>(c.g) << 8) |
           static_cast<uint32_t>(c.b);
}
