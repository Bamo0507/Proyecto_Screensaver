cmake_minimum_required(VERSION 3.16)
project(screensaver LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Release por defecto: medir speedup sin optimización no significa nada
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Tipo de build" FORCE)
endif()
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

# Apple Silicon: Homebrew vive en /opt/homebrew y Apple Clang no trae OpenMP
if(APPLE)
    execute_process(COMMAND brew --prefix
                    OUTPUT_VARIABLE PREFIJO_BREW
                    OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
    list(APPEND CMAKE_PREFIX_PATH "${PREFIJO_BREW}")

    execute_process(COMMAND brew --prefix libomp
                    OUTPUT_VARIABLE PREFIJO_LIBOMP
                    OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
    if(PREFIJO_LIBOMP)
        set(OpenMP_CXX_FLAGS     "-Xpreprocessor -fopenmp -I${PREFIJO_LIBOMP}/include")
        set(OpenMP_CXX_LIB_NAMES "omp")
        set(OpenMP_omp_LIBRARY   "${PREFIJO_LIBOMP}/lib/libomp.dylib")
    endif()
endif()

find_package(SDL2   REQUIRED)  # ventana, renderer, eventos y textura del fondo
find_package(OpenMP REQUIRED)  # paralelización por directivas #pragma omp

add_executable(screensaver
    src/main.cpp

    src/shared/config.cpp
    src/shared/palette.cpp
    src/shared/timing.cpp

    src/fondo/fondo.cpp
    src/fondo/fondo_seq.cpp
    src/fondo/fondo_par.cpp

    src/estrellas/estrellas.cpp

    src/constelacion/constelacion.cpp
    src/constelacion/aristas_seq.cpp
    src/constelacion/aristas_par.cpp

    src/fugaces/fugaces.cpp

    src/nave/nave.cpp
)

target_include_directories(screensaver PRIVATE ${CMAKE_SOURCE_DIR}/src)

if(TARGET SDL2::SDL2)
    target_link_libraries(screensaver PRIVATE SDL2::SDL2)
else()
    target_include_directories(screensaver PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(screensaver PRIVATE ${SDL2_LIBRARIES})
endif()

target_link_libraries(screensaver PRIVATE OpenMP::OpenMP_CXX)
