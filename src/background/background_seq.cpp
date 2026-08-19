#include "background/background.hpp"

#include <cmath>

#include "shared/palette.hpp"

namespace {

// Devuelve un valor continuo, no un color: así cualquier división futura del
// dominio conserva la misma imagen y no crea bandas entre secciones.
float backgroundField(float x, float y, float time) {
    const float diagonal = std::sin(x * 0.017f + y * 0.011f + time * 0.52f);
    const float waves = std::cos(y * 0.024f - time * 0.31f);
    const float swirl = std::sin((x + y) * 0.009f - time * 0.18f);
    return diagonal * 1.9f + waves * 1.35f + swirl * 0.85f;
}

} // namespace

namespace background {

void renderSequential(uint32_t* target,
                      int width,
                      int height,
                      float time,
                      const KernelParams& params) {
    (void)params; // El baseline no crea regiones OpenMP ni usa sus parámetros.

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const Color color = dim(fromPalette(backgroundField(
                                        static_cast<float>(x),
                                        static_cast<float>(y),
                                        time)),
                                    0.30f);
            target[y * width + x] = packPixel(color);
        }
    }
}

} // namespace background
