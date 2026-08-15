#pragma once

#include "shared/kernel.hpp"

// Toda la configuración de la corrida. Nada de esto se escribe en el código:
// todo entra por CLI.
struct Config {
    int cantidadNodos = 0; // N posicional, obligatorio
    bool paralelo = false; // --parallel
    int hilos = 0; // --hilos T (0 = todos los que reporte OpenMP)
    int secciones = 4; // --secciones S
    int trozo = 32; // --trozo C (chunk de schedule dynamic)
    float radio = 120.0f; // --radio R (distancia máxima para unir dos nodos)
    int ancho = 1280; // --ancho W
    int alto = 720; // --alto H
    unsigned semilla = 42; // --seed S
    int fondoEscala = 4; // --fondo-escala E (el fondo se calcula a 1/E y se estira)
    int framesBench = 0; // --bench F (0 = modo interactivo con vsync)
};

// Llena salida a partir de argv. Devuelve false si algo es inválido, ya
// habiendo impreso el error y el uso.
bool parsearArgumentos(int argc, char** argv, Config& salida);

void imprimirUso(const char* nombrePrograma);

// Empaqueta los campos de ejecución para pasárselos a los kernels.
KernelParams aKernelParams(const Config& cfg);
