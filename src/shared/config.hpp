#pragma once

#include "shared/kernel.hpp"

// Toda la configuración de la corrida. Nada de esto se escribe en el código:
// todo entra por CLI.
struct Config {
    int nodeCount = 0; // N posicional, obligatorio
    bool parallel = false; // --parallel
    int threads = 0; // --hilos T (0 = todos los que reporte OpenMP)
    int sections = 4; // --secciones S
    int chunk = 32; // --trozo C (bloque de schedule dynamic)
    float radius = 50.0f; // --radio R (distancia máxima para unir dos nodos)
    int width = 1280; // --ancho W
    int height = 720; // --alto H
    unsigned seed = 42; // --seed S
    int backgroundScale = 4; // --fondo-escala E (el fondo se calcula a 1/E y se estira)
    int benchFrames = 0; // --bench F (0 = modo interactivo con vsync)
};

// Ok: seguir. Help: se pidió --help, salir con 0. Error: salir con 1.
enum class ArgsResult { Ok, Help, Error };

// Llena out a partir de argv. Si algo es inválido imprime el motivo y el uso
// en stderr antes de devolver Error.
ArgsResult parseArguments(int argc, char** argv, Config& out);

void printUsage(const char* programName);

// Empaqueta los campos de ejecución para pasárselos a los kernels.
KernelParams makeKernelParams(const Config& cfg);
