#include "constellation/constellation.hpp"

#include <algorithm>
#include <cmath>

#include "shared/palette.hpp"

namespace {

// Rapidez de los nodos en píxeles por segundo. No entra por CLI porque no es
// un parámetro del experimento sino un ajuste estético: cambiarla no altera el
// costo del O(N^2) ni el reparto de trabajo entre hilos.
constexpr float SPEED_MIN = 18.0f;
constexpr float SPEED_MAX = 85.0f;

constexpr float TWO_PI = 6.28318530718f;

// Lado en píxeles del cuadrito con que se dibuja cada nodo.
constexpr int NODE_SIZE = 3;

// Opacidad de una arista cuando sus dos nodos están prácticamente encima.
constexpr float EDGE_MAX_ALPHA = 190.0f;

// Techo de la reserva inicial de aristas, en cantidad de elementos. Con N y
// radio grandes la estimación se dispara, y reservar de más costaría más
// memoria de la que el programa llegaría a usar.
constexpr size_t EDGE_RESERVE_CAP = 2000000;

// Estado de la capa en Structure of Arrays: un arreglo por atributo, en vez de
// un solo arreglo de structs Node.
//
// El motivo es la detección de aristas. Ese bucle lee posiciones y nada más;
// con un arreglo de structs, cada línea de caché traería también velocidades y
// colores que el bucle no toca, y se desperdiciaría más de la mitad del ancho
// de banda de memoria en el punto más caliente del programa.
std::vector<float> posX;
std::vector<float> posY;
std::vector<float> velX;
std::vector<float> velY;
std::vector<int> colorIndex;

// Aristas del frame actual. Vive aquí y no dentro de update para que su
// capacidad se reserve una sola vez y no se realoque 60 veces por segundo.
std::vector<Edge> edges;

Config config;
KernelParams kernelParams;
EdgeKernel edgeKernel = nullptr;
float radiusSquared = 0.0f;

} // namespace

namespace constellation {

void init(const Config& cfg, Rng& rng) {
    config = cfg;
    kernelParams = makeKernelParams(cfg);
    radiusSquared = cfg.radius * cfg.radius;

    const int count = cfg.nodeCount;
    posX.resize(count);
    posY.resize(count);
    velX.resize(count);
    velY.resize(count);
    colorIndex.resize(count);

    for (int i = 0; i < count; ++i) {
        posX[i] = randomFloat(rng, 0.0f, static_cast<float>(cfg.width));
        posY[i] = randomFloat(rng, 0.0f, static_cast<float>(cfg.height));

        // Se sortea ángulo y rapidez por separado, no velX y velY sueltas. Al
        // sortear las dos componentes de forma independiente, las diagonales
        // salen favorecidas y el movimiento del conjunto se ve sesgado.
        const float angle = randomFloat(rng, 0.0f, TWO_PI);
        const float speed = randomFloat(rng, SPEED_MIN, SPEED_MAX);
        velX[i] = std::cos(angle) * speed;
        velY[i] = std::sin(angle) * speed;

        // El color se fija al inicializar y ya no cambia: se sortea el índice
        // de la paleta, nunca un RGB libre.
        colorIndex[i] = randomIndex(rng, PALETTE_SIZE);
    }

    // Cuántas aristas esperar, para reservar de una vez. Dos nodos se unen si
    // el segundo cae dentro del círculo de radio r del primero, cuya área es
    // pi*r^2 sobre el área total de la ventana; multiplicado por los N*(N-1)/2
    // pares da el estimado. El 1.3 es margen porque los nodos no se distribuyen
    // perfectamente uniformes.
    const double area = static_cast<double>(cfg.width) * static_cast<double>(cfg.height);
    const double density = 3.14159265 * static_cast<double>(cfg.radius) * cfg.radius / area;
    const double expected = 0.5 * count * (count - 1) * density;
    const size_t reserve = static_cast<size_t>(std::min(expected * 1.3 + 64.0,
                                                        static_cast<double>(EDGE_RESERVE_CAP)));
    edges.reserve(reserve);

    // El escalón v2 conectará detectEdgesParallel aquí según cfg.parallel. Por
    // ahora la capa solo tiene kernel secuencial y --parallel no la afecta.
    edgeKernel = detectEdgesSequential;
}

void update(float dt) {
    const int count = config.nodeCount;
    const float maxX = static_cast<float>(config.width - 1);
    const float maxY = static_cast<float>(config.height - 1);

    for (int i = 0; i < count; ++i) {
        posX[i] += velX[i] * dt;
        posY[i] += velY[i] * dt;

        // Rebote por reflexión, no por recorte: si el nodo se pasó 5 px del
        // borde, entra 5 px de vuelta. Recortarlo a la orilla haría que los
        // nodos rápidos se quedaran pegados a la pared un frame de más, y en
        // conjunto se ve como una franja de nodos acumulados en el borde.
        if (posX[i] < 0.0f) {
            posX[i] = -posX[i];
            velX[i] = -velX[i];
        } else if (posX[i] > maxX) {
            posX[i] = 2.0f * maxX - posX[i];
            velX[i] = -velX[i];
        }

        if (posY[i] < 0.0f) {
            posY[i] = -posY[i];
            velY[i] = -velY[i];
        } else if (posY[i] > maxY) {
            posY[i] = 2.0f * maxY - posY[i];
            velY[i] = -velY[i];
        }
    }

    // Región paralela A. Hoy apunta al kernel secuencial.
    edgeKernel(posX.data(), posY.data(), count, radiusSquared, edges, kernelParams);
}

void draw(SDL_Renderer* renderer) {
    // Sin blending, el alpha de las aristas se ignora y todas saldrían opacas.
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // Las aristas van primero: quedan por debajo de los nodos.
    for (const Edge& edge : edges) {
        // Color intermedio entre los dos extremos. SDL dibuja la línea de un
        // solo color, así que el degradado real a lo largo del trazo no es
        // posible sin dibujarla por segmentos, que costaría mucho más.
        const Color color = blend(PALETTE[colorIndex[edge.nodeA]],
                                  PALETTE[colorIndex[edge.nodeB]],
                                  0.5f);

        // El alpha sale de closeness: mientras más cerca estén los dos nodos,
        // más sólida la línea. Las que están por romperse se ven casi como un
        // rastro, y eso es lo que da la sensación de constelación viva.
        const Uint8 alpha = static_cast<Uint8>(edge.closeness * EDGE_MAX_ALPHA);

        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, alpha);
        SDL_RenderDrawLine(renderer,
                           static_cast<int>(posX[edge.nodeA]),
                           static_cast<int>(posY[edge.nodeA]),
                           static_cast<int>(posX[edge.nodeB]),
                           static_cast<int>(posY[edge.nodeB]));
    }

    const int count = config.nodeCount;
    for (int i = 0; i < count; ++i) {
        const Color color = PALETTE[colorIndex[i]];
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);

        SDL_Rect box;
        box.x = static_cast<int>(posX[i]) - NODE_SIZE / 2;
        box.y = static_cast<int>(posY[i]) - NODE_SIZE / 2;
        box.w = NODE_SIZE;
        box.h = NODE_SIZE;
        SDL_RenderFillRect(renderer, &box);
    }
}

void destroy() {
    // clear() por sí solo no devuelve la memoria: hay que soltar la capacidad.
    posX.clear();
    posX.shrink_to_fit();
    posY.clear();
    posY.shrink_to_fit();
    velX.clear();
    velX.shrink_to_fit();
    velY.clear();
    velY.shrink_to_fit();
    colorIndex.clear();
    colorIndex.shrink_to_fit();
    edges.clear();
    edges.shrink_to_fit();

    edgeKernel = nullptr;
}

} // namespace constellation
