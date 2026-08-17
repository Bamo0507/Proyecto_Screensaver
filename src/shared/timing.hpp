#pragma once

#include "shared/config.hpp"

namespace timing {

// Las tres regiones se miden por separado y nunca promediadas: escalan muy
// distinto y promediarlas esconde el análisis.
enum Region {
    REGION_BACKGROUND = 0, // campo de fondo, O(W*H)
    REGION_EDGES,          // detección de aristas, O(N^2)
    REGION_RENDER,         // dibujo con SDL, siempre serial
    REGION_COUNT
};

// En modo --bench imprime el encabezado del CSV a stdout.
void init(const Config& cfg);

void beginFrame();
void begin(Region region);
void end(Region region);

// Cierra el frame, actualiza los FPS y, en --bench, emite la fila del CSV.
void endFrame();

// Media móvil exponencial de los FPS: suaviza sin necesitar acumuladores ni
// una ventana de tiempo.
double smoothedFps();

// Duración del último frame completo, en segundos.
double lastFrameSeconds();

void destroy();

} // namespace timing
