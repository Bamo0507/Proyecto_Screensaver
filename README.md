# Screensaver — Constelación dinámica

Screensaver en C++ con SDL2 y OpenMP.
Bryan Martínez · Adriana Palacios

N nodos se mueven por la pantalla, rebotan en los bordes y se conectan con
líneas cuando quedan a menos de cierta distancia. El mismo programa contiene la
versión secuencial y la paralela; la bandera `--parallel` elige cuál correr.

## Dependencias

Solo SDL2 y OpenMP.

**macOS** — Apple Clang no trae OpenMP, se usa la libomp de Homebrew:

```bash
brew install sdl2 libomp
```

**Linux (Debian/Ubuntu)** — OpenMP viene con GCC:

```bash
sudo apt install libsdl2-dev cmake build-essential
```

## Compilar

```bash
cmake -B build
cmake --build build
```

El build sale en Release con `-O2` por defecto. **No compiles en Debug para
medir**: sin optimización el binario corre varias veces más lento y los
speedups no significan nada.

Si agregas un `.cpp` nuevo, hay que listarlo en `CMakeLists.txt` — la lista de
fuentes es explícita, no un glob.

## Correr

```bash
./build/screensaver N [opciones]
```

`N` es la cantidad de nodos, es posicional y es obligatorio.

```bash
./build/screensaver 2000                          # secuencial
./build/screensaver 2000 --parallel               # paralelo
./build/screensaver 2000 --parallel --hilos 4     # paralelo con 4 hilos
./build/screensaver 2000 --radio 80 --seed 7      # más conexiones, otra escena
./build/screensaver 5000 --parallel --bench 600   # 600 frames sin vsync
```

Se cierra con `ESC` o con la X de la ventana. El título muestra N, el radio, el
modo y los FPS.

## Opciones

| Bandera | Rango | Por defecto | Qué hace |
|---|---|---|---|
| `N` | 1–100000 | — | cantidad de nodos (obligatorio) |
| `--parallel` | — | apagado | usa los kernels paralelos |
| `--hilos T` | 1–256 | los del sistema | hilos de OpenMP |
| `--secciones S` | 1–64 | 4 | franjas de filas en que se parte el fondo |
| `--trozo C` | 1–65536 | 32 | bloque de `schedule(dynamic)` en aristas |
| `--radio R` | 0.1–10000 | 50 | distancia máxima para unir dos nodos |
| `--ancho W` | 160–7680 | 1280 | ancho de la ventana |
| `--alto H` | 120–4320 | 720 | alto de la ventana |
| `--seed S` | 0–4294967295 | 42 | semilla del generador |
| `--fondo-escala E` | 1–32 | 4 | el fondo se calcula a 1/E y se estira |
| `--bench F` | 1–1000000 | — | corre F frames sin vsync y termina |
| `--help`, `-h` | — | — | muestra la ayuda |

Todo argumento inválido — no numérico, cero, negativo, fuera de rango, faltante
o mal escrito — imprime el motivo y el uso, y sale con código distinto de cero.

**`--radio` cambia los resultados.** Las aristas escalan con el cuadrado del
radio: con N=2000, radio 50 da ~16 mil aristas y radio 120 da ~89 mil. Cualquier
número que reportes tiene que ir acompañado del radio con que se midió.
