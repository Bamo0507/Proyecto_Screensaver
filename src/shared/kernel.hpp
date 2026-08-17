#pragma once

#include <cstdint>
#include <vector>

// Se declara adelantado para no depender de constellation.hpp: shared/ no debe
// conocer a las capas, solo al revés.
struct Edge;

// Parámetros de EJECUCIÓN (cómo correr), no de datos (sobre qué correr).
// Salen del CLI y son iguales para las dos regiones paralelas.
struct KernelParams {
    int threads = 1; // --hilos: cuántos hilos pide OpenMP
    int sections = 4; // --secciones: en cuántas franjas de filas se parte el fondo
    int chunk = 32; // tamaño de bloque para schedule(dynamic, chunk) en aristas
};

// Región B, campo de fondo.
// Escribe width*height pixeles en target. Escrituras disjuntas, cero sincronía.
using BackgroundKernel = void (*)(uint32_t* target,
                                  int width,
                                  int height,
                                  float time,
                                  const KernelParams& params);

// Región A, detección de aristas.
// Recorre los pares (i,j) y acumula en out los que quedan bajo el radio.
//
// Recibe las posiciones como dos arreglos sueltos y no como un arreglo de
// nodos: el bucle O(N^2) solo necesita X e Y, así que pasándolos por separado
// cada línea de caché que sube trae puras coordenadas, sin velocidades ni
// colores que el bucle no va a mirar.
//
// radiusSquared llega ya elevado al cuadrado para evitar sqrt dentro del O(N^2).
using EdgeKernel = void (*)(const float* posX,
                            const float* posY,
                            int count,
                            float radiusSquared,
                            std::vector<Edge>& out,
                            const KernelParams& params);
