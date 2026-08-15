# Screensaver — Constelación dinámica

Screensaver en C++ con SDL2 y OpenMP.
Bryan Martínez · Adriana Palacios

## Dependencias

macOS (Apple Clang no trae OpenMP, se usa la libomp de Homebrew):

```bash
brew install sdl2 libomp
```

## Compilar

```bash
cmake -B build
cmake --build build
```

## Correr

```bash
./build/screensaver
```

Se cierra con `ESC` o con la X de la ventana.
