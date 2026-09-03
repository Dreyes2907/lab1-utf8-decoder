# Laboratorio 1 - Decodificador UTF-8

Programa en C++ que decodifica manualmente archivos de texto en UTF-8, byte por byte, sin usar ninguna librería de codificación de Unicode.

## Requisitos

Necesitas GCC 13 o más nuevo (por `std::format`). Para revisar tu versión:

```bash
g++ --version
```

## Cómo compilar

Con CMake:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

El ejecutable queda en `build/utf8_decoder`.

También se puede compilar directo con g++, sin CMake:

```bash
g++ -std=c++20 -Wall -Wextra -Wpedantic src/main.cpp -o utf8_decoder
```

## Cómo ejecutar

El programa recibe el archivo a decodificar como argumento:

```bash
./build/utf8_decoder archivo.txt
```

## Qué imprime

Primero el contenido decodificado (los caracteres imprimibles tal cual, y todo lo demás como `U+XXXX`), luego los errores que haya encontrado con su offset, y al final un resumen con el total de bytes, cuántos code points de cada tamaño (1 a 4 bytes) y cuántos errores hubo.

Si encuentra un byte mal formado no detiene el programa, sigue leyendo el resto del archivo y reporta el error al final.
