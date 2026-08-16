#include "shared/config.hpp"

#include <cctype>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace {

// Rangos válidos de cada parámetro. Se declaran juntos para que el mensaje de
// error y la validación no puedan desincronizarse.
constexpr int NODES_MIN = 1;
constexpr int NODES_MAX = 100000; // arriba de esto el O(N^2) deja de correr en tiempo real
constexpr int THREADS_MIN = 1;
constexpr int THREADS_MAX = 256;
constexpr int SECTIONS_MIN = 1;
constexpr int SECTIONS_MAX = 64;
constexpr int CHUNK_MIN = 1;
constexpr int CHUNK_MAX = 65536;
constexpr int WIDTH_MIN = 160;
constexpr int WIDTH_MAX = 7680;
constexpr int HEIGHT_MIN = 120;
constexpr int HEIGHT_MAX = 4320;
constexpr int SCALE_MIN = 1;
constexpr int SCALE_MAX = 32;
constexpr int BENCH_MIN = 1;
constexpr int BENCH_MAX = 1000000;
constexpr float RADIUS_MIN = 0.1f;
constexpr float RADIUS_MAX = 10000.0f;
constexpr long SEED_MAX = 4294967295L;

// Convierte texto a entero exigiendo que TODO el texto sea el número.
// strtol por sí solo acepta "12abc" y devuelve 12; aquí eso es un error.
bool textToInteger(const char* text, long& out) {
    if (text == nullptr || *text == '\0') {
        return false;
    }
    errno = 0;
    char* conversionEnd = nullptr;
    const long value = std::strtol(text, &conversionEnd, 10);
    if (errno == ERANGE || conversionEnd == text || *conversionEnd != '\0') {
        return false;
    }
    out = value;
    return true;
}

bool textToFloat(const char* text, float& out) {
    if (text == nullptr || *text == '\0') {
        return false;
    }
    errno = 0;
    char* conversionEnd = nullptr;
    const float value = std::strtof(text, &conversionEnd);
    if (errno == ERANGE || conversionEnd == text || *conversionEnd != '\0') {
        return false;
    }
    out = value;
    return true;
}

// Toma el valor que sigue a una bandera y avanza el índice.
// Devuelve nullptr si no hay valor, o si lo que sigue es otra bandera
// (el caso "--hilos --parallel", donde el usuario olvidó el número).
const char* flagValue(int argc, char** argv, int& index, const char* flag) {
    if (index + 1 >= argc) {
        std::fprintf(stderr, "Error: la bandera %s requiere un valor.\n", flag);
        return nullptr;
    }
    const char* candidate = argv[index + 1];
    if (candidate[0] == '-' && candidate[1] == '-') {
        std::fprintf(stderr, "Error: la bandera %s requiere un valor, pero le sigue %s.\n",
                     flag, candidate);
        return nullptr;
    }
    ++index;
    return candidate;
}

bool readIntInRange(const char* flag, const char* text, int minimum, int maximum, int& out) {
    long value = 0;
    if (!textToInteger(text, value)) {
        std::fprintf(stderr, "Error: %s espera un numero entero, se recibio '%s'.\n", flag, text);
        return false;
    }
    if (value < minimum || value > maximum) {
        std::fprintf(stderr, "Error: %s debe estar entre %d y %d, se recibio %ld.\n",
                     flag, minimum, maximum, value);
        return false;
    }
    out = static_cast<int>(value);
    return true;
}

bool readFloatInRange(const char* flag, const char* text, float minimum, float maximum, float& out) {
    float value = 0.0f;
    if (!textToFloat(text, value)) {
        std::fprintf(stderr, "Error: %s espera un numero, se recibio '%s'.\n", flag, text);
        return false;
    }
    if (value < minimum || value > maximum) {
        std::fprintf(stderr, "Error: %s debe estar entre %.2f y %.2f, se recibio %.2f.\n",
                     flag, minimum, maximum, value);
        return false;
    }
    out = value;
    return true;
}

// La semilla es el unico parametro donde 0 es valido, por eso no reusa
// readIntInRange: ahi el minimo siempre es 1.
bool readSeed(const char* text, unsigned& out) {
    long value = 0;
    if (!textToInteger(text, value)) {
        std::fprintf(stderr, "Error: --seed espera un numero entero, se recibio '%s'.\n", text);
        return false;
    }
    if (value < 0 || value > SEED_MAX) {
        std::fprintf(stderr, "Error: --seed debe estar entre 0 y %ld, se recibio %ld.\n",
                     SEED_MAX, value);
        return false;
    }
    out = static_cast<unsigned>(value);
    return true;
}

} // namespace

void printUsage(const char* programName) {
    std::fprintf(stderr,
        "\nUso: %s N [opciones]\n"
        "\n"
        "  N                 cantidad de nodos de la constelacion (%d-%d, obligatorio)\n"
        "\n"
        "Opciones:\n"
        "  --parallel        usa los kernels paralelos en vez de los secuenciales\n"
        "  --hilos T         hilos de OpenMP (%d-%d, por defecto los que reporte el sistema)\n"
        "  --secciones S     franjas de filas en que se parte el fondo (%d-%d, por defecto 4)\n"
        "  --trozo C         bloque de schedule(dynamic) en aristas (%d-%d, por defecto 32)\n"
        "  --radio R         distancia maxima para unir dos nodos (%.1f-%.1f, por defecto 120)\n"
        "  --ancho W         ancho de la ventana (%d-%d, por defecto 1280)\n"
        "  --alto H          alto de la ventana (%d-%d, por defecto 720)\n"
        "  --seed S          semilla del generador (0-%ld, por defecto 42)\n"
        "  --fondo-escala E  el fondo se calcula a 1/E y se estira (%d-%d, por defecto 4)\n"
        "  --bench F         corre F frames sin vsync y saca CSV (%d-%d)\n"
        "  --help, -h        muestra esta ayuda\n"
        "\n"
        "Ejemplos:\n"
        "  %s 2000\n"
        "  %s 2000 --parallel --hilos 8\n"
        "  %s 5000 --parallel --bench 600 --seed 7\n"
        "\n",
        programName,
        NODES_MIN, NODES_MAX,
        THREADS_MIN, THREADS_MAX,
        SECTIONS_MIN, SECTIONS_MAX,
        CHUNK_MIN, CHUNK_MAX,
        RADIUS_MIN, RADIUS_MAX,
        WIDTH_MIN, WIDTH_MAX,
        HEIGHT_MIN, HEIGHT_MAX,
        SEED_MAX,
        SCALE_MIN, SCALE_MAX,
        BENCH_MIN, BENCH_MAX,
        programName, programName, programName);
}

ArgsResult parseArguments(int argc, char** argv, Config& out) {
    const char* programName = (argc > 0) ? argv[0] : "screensaver";
    bool explicitThreads = false;
    bool hasNodeCount = false;

    for (int i = 1; i < argc; ++i) {
        const char* arg = argv[i];

        if (std::strcmp(arg, "--help") == 0 || std::strcmp(arg, "-h") == 0) {
            printUsage(programName);
            return ArgsResult::Help;
        }

        if (std::strcmp(arg, "--parallel") == 0) {
            out.parallel = true;
            continue;
        }

        if (std::strcmp(arg, "--hilos") == 0) {
            const char* value = flagValue(argc, argv, i, arg);
            if (!value || !readIntInRange(arg, value, THREADS_MIN, THREADS_MAX, out.threads)) {
                printUsage(programName);
                return ArgsResult::Error;
            }
            explicitThreads = true;
            continue;
        }

        if (std::strcmp(arg, "--secciones") == 0) {
            const char* value = flagValue(argc, argv, i, arg);
            if (!value || !readIntInRange(arg, value, SECTIONS_MIN, SECTIONS_MAX, out.sections)) {
                printUsage(programName);
                return ArgsResult::Error;
            }
            continue;
        }

        if (std::strcmp(arg, "--trozo") == 0) {
            const char* value = flagValue(argc, argv, i, arg);
            if (!value || !readIntInRange(arg, value, CHUNK_MIN, CHUNK_MAX, out.chunk)) {
                printUsage(programName);
                return ArgsResult::Error;
            }
            continue;
        }

        if (std::strcmp(arg, "--radio") == 0) {
            const char* value = flagValue(argc, argv, i, arg);
            if (!value || !readFloatInRange(arg, value, RADIUS_MIN, RADIUS_MAX, out.radius)) {
                printUsage(programName);
                return ArgsResult::Error;
            }
            continue;
        }

        if (std::strcmp(arg, "--ancho") == 0) {
            const char* value = flagValue(argc, argv, i, arg);
            if (!value || !readIntInRange(arg, value, WIDTH_MIN, WIDTH_MAX, out.width)) {
                printUsage(programName);
                return ArgsResult::Error;
            }
            continue;
        }

        if (std::strcmp(arg, "--alto") == 0) {
            const char* value = flagValue(argc, argv, i, arg);
            if (!value || !readIntInRange(arg, value, HEIGHT_MIN, HEIGHT_MAX, out.height)) {
                printUsage(programName);
                return ArgsResult::Error;
            }
            continue;
        }

        if (std::strcmp(arg, "--seed") == 0) {
            const char* value = flagValue(argc, argv, i, arg);
            if (!value || !readSeed(value, out.seed)) {
                printUsage(programName);
                return ArgsResult::Error;
            }
            continue;
        }

        if (std::strcmp(arg, "--fondo-escala") == 0) {
            const char* value = flagValue(argc, argv, i, arg);
            if (!value || !readIntInRange(arg, value, SCALE_MIN, SCALE_MAX, out.backgroundScale)) {
                printUsage(programName);
                return ArgsResult::Error;
            }
            continue;
        }

        if (std::strcmp(arg, "--bench") == 0) {
            const char* value = flagValue(argc, argv, i, arg);
            if (!value || !readIntInRange(arg, value, BENCH_MIN, BENCH_MAX, out.benchFrames)) {
                printUsage(programName);
                return ArgsResult::Error;
            }
            continue;
        }

        // Cualquier cosa que empiece con guion y no haya calzado arriba es una
        // bandera mal escrita. Ignorarla en silencio seria peor: el usuario
        // creeria que corrio con --parallel cuando escribio --paralel.
        // Se excluyen los numeros negativos ("-5"), que son un intento de dar N
        // y merecen el mensaje de rango, no el de bandera desconocida.
        const bool isNegativeNumber =
            (arg[0] == '-') && (std::isdigit(static_cast<unsigned char>(arg[1])) != 0);
        if (arg[0] == '-' && !isNegativeNumber) {
            std::fprintf(stderr, "Error: bandera desconocida '%s'.\n", arg);
            printUsage(programName);
            return ArgsResult::Error;
        }

        // Lo unico posicional es N.
        if (hasNodeCount) {
            std::fprintf(stderr, "Error: N ya se habia especificado, sobra el argumento '%s'.\n", arg);
            printUsage(programName);
            return ArgsResult::Error;
        }
        if (!readIntInRange("N", arg, NODES_MIN, NODES_MAX, out.nodeCount)) {
            printUsage(programName);
            return ArgsResult::Error;
        }
        hasNodeCount = true;
    }

    if (!hasNodeCount) {
        std::fprintf(stderr, "Error: falta N, la cantidad de nodos.\n");
        printUsage(programName);
        return ArgsResult::Error;
    }

    // Coherencia entre banderas: pedir hilos sin paralelo no rompe nada, pero
    // el usuario creeria estar midiendo la version paralela y no lo estaria.
    if (explicitThreads && !out.parallel) {
        std::fprintf(stderr,
                     "Aviso: --hilos %d no tiene efecto sin --parallel; se corre en secuencial.\n",
                     out.threads);
    }

    // El radio mas grande que la ventana une a todos con todos: N^2 aristas.
    const float windowSpan = static_cast<float>(out.width) + static_cast<float>(out.height);
    if (out.radius > windowSpan) {
        std::fprintf(stderr,
                     "Aviso: --radio %.1f supera el tamano de la ventana; se uniran casi todos los pares.\n",
                     out.radius);
    }

    return ArgsResult::Ok;
}

KernelParams makeKernelParams(const Config& cfg) {
    KernelParams params;
    params.threads = cfg.threads;
    params.sections = cfg.sections;
    params.chunk = cfg.chunk;
    return params;
}
