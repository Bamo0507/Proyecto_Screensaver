#pragma once

#include <cstdint>
#include <vector>

struct Nodo;
struct Arista;

// Parámetros de EJECUCIÓN (cómo correr), no de datos (sobre qué correr).
struct KernelParams {
    int hilos = 1;   // --hilos: cuántos hilos pide OpenMP
    int secciones = 4;   // --secciones: en cuántas franjas de filas se parte el fondo
    int trozo = 32;  // tamaño de chunk para schedule(dynamic, trozo) en aristas
};

// Escribe ancho*alto pixeles en destino. Escrituras disjuntas, cero sincronía.
using KernelFondo = void (*)(uint32_t* destino,
                             int ancho,
                             int alto,
                             float tiempo,
                             const KernelParams& kp);

// Recorre los pares (i,j) y acumula en salida los que quedan bajo el radio.
using KernelAristas = void (*)(const Nodo* nodos,
                               int cantidad,
                               float radio2,
                               std::vector<Arista>& salida,
                               const KernelParams& kp);
