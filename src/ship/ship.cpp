#include "ship/ship.hpp"

#include <cmath>

#include "shared/palette.hpp"

namespace {

// Rapidez de la nave en píxeles por segundo. Va más rápido que los nodos a
// propósito: es el único elemento que cruza la escena entera y tiene que
// leerse como un objeto que viaja, no como una partícula más.
constexpr float SPEED_MIN = 130.0f;
constexpr float SPEED_MAX = 230.0f;

// Geometría del casco, medida desde el centro de la nave. El morro se estira
// bastante más de lo que sobresale la cola para que la silueta apunte con
// claridad hacia donde va.
constexpr float NOSE_LENGTH = 22.0f;
constexpr float TAIL_LENGTH = 11.0f;
constexpr float HALF_WIDTH = 10.0f;

// Alerones: dos triángulos pequeños pegados a la cola que ensanchan la silueta
// y la hacen legible aunque la nave sea chica en pantalla.
constexpr float FIN_SPAN = 17.0f;
constexpr float FIN_LENGTH = 9.0f;

// Propulsores. Salen dos llamas, una por alerón, y cada una se dibuja en dos
// capas: una externa larga y translúcida, y un núcleo corto y sólido.
constexpr float FLAME_BASE = 30.0f;
constexpr float FLAME_HALF_WIDTH = 4.5f;
constexpr float FLAME_OFFSET = 5.5f; // separación vertical entre las dos llamas
constexpr Uint8 FLAME_OUTER_ALPHA = 95;
constexpr Uint8 FLAME_CORE_ALPHA = 225;

// Frecuencias del parpadeo del propulsor. Son dos y no una, y deliberadamente
// no múltiplos entre sí: si fuera una sola senoidal el titileo se vería
// mecánico, como un pulso regular. Al sumar dos ciclos que nunca coinciden, el
// patrón tarda mucho en repetirse y se lee como combustión.
constexpr float FLICKER_FAST = 27.0f;
constexpr float FLICKER_SLOW = 41.0f;

// Cuánto tiene que alejarse del borde antes de reaparecer. Sin este margen la
// nave saltaría estando todavía a la vista, y se vería como un parpadeo en vez
// de como una salida limpia de cuadro.
constexpr float EXIT_MARGIN = NOSE_LENGTH + FLAME_BASE + 10.0f;

// La nave es un solo objeto, así que su estado son variables sueltas y no
// arreglos: aquí no hay nada que recorrer ni que paralelizar.
float posX = 0.0f;
float posY = 0.0f;
float speed = 0.0f;

// Tiempo acumulado, solo para animar la llama.
float flamePhase = 0.0f;

int colorIndex = 0;

Config config;

// Puntero al generador compartido de main, que vive más que esta capa. Hace
// falta guardarlo porque cada reaparición sortea una altura nueva, y eso ocurre
// en update, donde el contrato de capa ya no recibe el Rng.
Rng* generator = nullptr;

// Coloca la nave a la izquierda, fuera de cuadro, a una altura al azar.
void spawn() {
    posX = -EXIT_MARGIN;
    posY = randomFloat(*generator, HALF_WIDTH, static_cast<float>(config.height) - HALF_WIDTH);
    speed = randomFloat(*generator, SPEED_MIN, SPEED_MAX);
}

// Triángulo sólido. SDL_RenderDrawLines solo daría el contorno.
void fillTriangle(SDL_Renderer* renderer,
                  float ax, float ay,
                  float bx, float by,
                  float cx, float cy,
                  Color color, Uint8 alpha) {
    const SDL_Color sdlColor{color.r, color.g, color.b, alpha};

    SDL_Vertex vertices[3];
    vertices[0].position = SDL_FPoint{ax, ay};
    vertices[1].position = SDL_FPoint{bx, by};
    vertices[2].position = SDL_FPoint{cx, cy};
    for (int i = 0; i < 3; ++i) {
        vertices[i].color = sdlColor;
        vertices[i].tex_coord = SDL_FPoint{0.0f, 0.0f};
    }

    SDL_RenderGeometry(renderer, nullptr, vertices, 3, nullptr, 0);
}

} // namespace

namespace ship {

void init(const Config& cfg, Rng& rng) {
    config = cfg;
    generator = &rng;

    colorIndex = randomIndex(rng, PALETTE_SIZE);
    flamePhase = 0.0f;
    spawn();
}

void update(float dt) {
    // Vuelo horizontal de izquierda a derecha, en línea recta: la altura se
    // sortea al aparecer y ya no cambia durante la travesía.
    posX += speed * dt;
    flamePhase += dt;

    // Al salir por la derecha vuelve a entrar por la izquierda, con altura y
    // rapidez nuevas. Es lo contrario del rebote de los nodos, y por eso la
    // nave da la sensación de estar de paso y no de estar encerrada.
    if (posX > static_cast<float>(config.width) + EXIT_MARGIN) {
        spawn();

        // Rota al siguiente color de la paleta en cada reaparición. Se avanza
        // en orden y no se sortea al azar por dos razones: así nunca le toca
        // dos veces seguidas el mismo tono, y la nave termina recorriendo la
        // paleta completa en vez de quedarse rondando dos o tres colores.
        colorIndex = (colorIndex + 1) % PALETTE_SIZE;
    }
}

void draw(SDL_Renderer* renderer) {
    // Las llamas llevan alpha, así que la capa activa el blending por su cuenta
    // en vez de confiar en que otra lo haya dejado puesto.
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    const Color hullColor = PALETTE[colorIndex];

    // La llama se aclara hacia el tono más luminoso de la paleta, para que
    // contraste con el casco sin salirse de la identidad de color.
    const Color flameColor = blend(hullColor, PALETTE[PALETTE_SIZE - 1], 0.75f);

    // Titileo del propulsor: dos senoidales de frecuencias que no encajan.
    const float flicker = 0.70f +
                          0.20f * std::sin(flamePhase * FLICKER_FAST) +
                          0.10f * std::sin(flamePhase * FLICKER_SLOW);
    const float outerLength = FLAME_BASE * flicker;
    const float coreLength = outerLength * 0.5f;

    const float tailX = posX - TAIL_LENGTH;

    // Propulsores: dos llamas simétricas respecto al eje de la nave.
    for (int side = -1; side <= 1; side += 2) {
        const float centerY = posY + side * FLAME_OFFSET;

        fillTriangle(renderer,
                     tailX, centerY - FLAME_HALF_WIDTH,
                     tailX, centerY + FLAME_HALF_WIDTH,
                     tailX - outerLength, centerY,
                     flameColor, FLAME_OUTER_ALPHA);

        fillTriangle(renderer,
                     tailX, centerY - FLAME_HALF_WIDTH * 0.55f,
                     tailX, centerY + FLAME_HALF_WIDTH * 0.55f,
                     tailX - coreLength, centerY,
                     flameColor, FLAME_CORE_ALPHA);
    }

    // Alerones, antes del casco para que este quede encima.
    const Color finColor = dim(hullColor, 0.65f);
    fillTriangle(renderer,
                 tailX + FIN_LENGTH, posY - HALF_WIDTH * 0.5f,
                 tailX, posY - FIN_SPAN,
                 tailX - FIN_LENGTH * 0.4f, posY - HALF_WIDTH * 0.5f,
                 finColor, 255);
    fillTriangle(renderer,
                 tailX + FIN_LENGTH, posY + HALF_WIDTH * 0.5f,
                 tailX, posY + FIN_SPAN,
                 tailX - FIN_LENGTH * 0.4f, posY + HALF_WIDTH * 0.5f,
                 finColor, 255);

    // Casco sólido.
    fillTriangle(renderer,
                 posX + NOSE_LENGTH, posY,
                 tailX, posY - HALF_WIDTH,
                 tailX, posY + HALF_WIDTH,
                 hullColor, 255);

    // Franja clara sobre el casco: le da volumen y evita que se lea como una
    // mancha plana del color de la paleta.
    const Color highlight = blend(hullColor, PALETTE[PALETTE_SIZE - 1], 0.6f);
    fillTriangle(renderer,
                 posX + NOSE_LENGTH * 0.75f, posY,
                 tailX + TAIL_LENGTH * 0.4f, posY - HALF_WIDTH * 0.38f,
                 tailX + TAIL_LENGTH * 0.4f, posY + HALF_WIDTH * 0.38f,
                 highlight, 255);
}

void destroy() {
    // No hay nada que liberar: la capa no reserva memoria dinámica ni texturas.
    // Se suelta el puntero al generador, que es de main y no de esta capa, y se
    // deja el estado en cero para que un init posterior no herede la posición
    // de una corrida anterior.
    generator = nullptr;
    posX = 0.0f;
    posY = 0.0f;
    speed = 0.0f;
    flamePhase = 0.0f;
    colorIndex = 0;
}

} // namespace ship
