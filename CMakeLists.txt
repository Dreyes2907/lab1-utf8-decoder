cmake_minimum_required(VERSION 3.20)
project(utf8_decoder LANGUAGES CXX)

# --- Requisito obligatorio del laboratorio: C++20 con soporte completo de <format> ---
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)   # usar C++20 puro, sin extensiones de GNU

# Ejecutable principal
add_executable(utf8_decoder
    src/main.cpp
)

# Mensaje útil al configurar, para verificar rápido qué compilador se está usando
message(STATUS "Compilador: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")

# --- Advertencias estrictas (buena práctica, ayuda a detectar bugs de bits/máscaras) ---
if (MSVC)
    target_compile_options(utf8_decoder PRIVATE /W4 /permissive-)
else()
    target_compile_options(utf8_decoder PRIVATE -Wall -Wextra -Wpedantic)
endif()

# --- (Opcional, para cuando agreguemos unit tests con doctest/Catch2) ---
# enable_testing()
# add_subdirectory(tests)