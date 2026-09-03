// Laboratorio 1 - Decodificador UTF-8
// Estructura de Datos II

#include <iostream>
#include <fstream>
#include <vector>
#include <optional>
#include <cstdint>
#include <string>
#include <format>

struct ErrorDecodificacion {
    size_t offset;
    std::string mensaje;
};

struct CodePointInfo {
    uint32_t valor;
    int bytesUsados;  // 1, 2, 3 o 4,  usado para el resumen final 
};


// Fase 1: Lectura del archivo en modo binario
std::optional<std::vector<uint8_t>> leerArchivoBinario(const std::string& ruta) {
    std::ifstream archivo(ruta, std::ios::binary);

    if (!archivo) {
        std::cerr << "Error: no se pudo abrir el archivo: " << ruta << "\n";
        return std::nullopt;
    }

    archivo.seekg(0, std::ios::end);
    std::streamsize tamanoBytes = archivo.tellg();
    archivo.seekg(0, std::ios::beg);

    if (tamanoBytes < 0) {
        std::cerr << "Error: no se pudo determinar el tamaño del archivo.\n";
        return std::nullopt;
    }

    if (tamanoBytes == 0) {
        return std::vector<uint8_t>{};  // archivo vacío: éxito, no error
    }

    std::vector<uint8_t> buffer(static_cast<size_t>(tamanoBytes));

    if (!archivo.read(reinterpret_cast<char*>(buffer.data()), tamanoBytes)) {
        std::cerr << "Error: fallo al leer el contenido del archivo.\n";
        return std::nullopt;
    }

    return buffer;
}


// Fase 2: Detección de BOM 
size_t detectarInicioTrasBOM(const std::vector<uint8_t>& buffer) {
    if (buffer.size() >= 3 &&
        buffer[0] == 0xEF &&
        buffer[1] == 0xBB &&
        buffer[2] == 0xBF) {
        return 3;
    }
    return 0;
}

// Un byte de continuación válido tiene el patrón 10xxxxxx
bool esByteContinuacionValido(uint8_t b) {
    return (b & 0xC0) == 0x80;
}


// Fase 3: Decodificación 

void decodificarUTF8(const std::vector<uint8_t>& buffer,
                      std::vector<CodePointInfo>& codePoints,
                      std::vector<ErrorDecodificacion>& errores) {
    size_t offset = detectarInicioTrasBOM(buffer);
    const size_t longitud = buffer.size();

    while (offset < longitud) {
        uint8_t b1 = buffer[offset];

        // Caso 1: ASCII puro (0xxxxxxx)
        if ((b1 & 0x80) == 0x00) {
            codePoints.push_back({static_cast<uint32_t>(b1), 1});
            offset += 1;
            continue;
        }

        // Caso 2: secuencia de 2 bytes (110xxxxx)
        if ((b1 & 0xE0) == 0xC0) {
            if (offset + 1 >= longitud) {
                errores.push_back({offset, "Secuencia incompleta: se esperaba 1 byte de continuación, EOF alcanzado."});
                offset += 1;
                continue;
            }
            uint8_t b2 = buffer[offset + 1];
            if (!esByteContinuacionValido(b2)) {
                errores.push_back({offset, "Byte de continuación esperado, no encontrado."});
                offset += 1;
                continue;
            }
            uint32_t codePoint = (static_cast<uint32_t>(b1 & 0x1F) << 6) |
                                  (static_cast<uint32_t>(b2 & 0x3F));

            // detección de codificación sobrelarga.
            if (codePoint < 0x80) {
                errores.push_back({offset, "Codificación sobrelarga (overlong encoding): el valor cabría en menos bytes."});
                offset += 2;  // la secuencia sí es estructuralmente válida, se consume completa
                continue;
            }

            codePoints.push_back({codePoint, 2});
            offset += 2;
            continue;
        }

        // Caso 3: secuencia de 3 bytes (1110xxxx)
        if ((b1 & 0xF0) == 0xE0) {
            if (offset + 2 >= longitud) {
                errores.push_back({offset, "Secuencia incompleta: se esperaban 2 bytes de continuación, EOF alcanzado."});
                offset += 1;
                continue;
            }
            uint8_t b2 = buffer[offset + 1];
            uint8_t b3 = buffer[offset + 2];
            if (!esByteContinuacionValido(b2) || !esByteContinuacionValido(b3)) {
                errores.push_back({offset, "Byte de continuación esperado, no encontrado."});
                offset += 1;
                continue;
            }
            uint32_t codePoint = (static_cast<uint32_t>(b1 & 0x0F) << 12) |
                                  (static_cast<uint32_t>(b2 & 0x3F) << 6) |
                                  (static_cast<uint32_t>(b3 & 0x3F));

            // overlong encoding para secuencias de 3 bytes (mínimo válido: 0x800)
            if (codePoint < 0x800) {
                errores.push_back({offset, "Codificación sobrelarga (overlong encoding): el valor cabría en menos bytes."});
                offset += 3;
                continue;
            }

            codePoints.push_back({codePoint, 3});
            offset += 3;
            continue;
        }

        // Caso 4: secuencia de 4 bytes (11110xxx) 
        if ((b1 & 0xF8) == 0xF0) {
            if (offset + 3 >= longitud) {
                errores.push_back({offset, "Secuencia incompleta: se esperaban 3 bytes de continuación, EOF alcanzado."});
                offset += 1;
                continue;
            }
            uint8_t b2 = buffer[offset + 1];
            uint8_t b3 = buffer[offset + 2];
            uint8_t b4 = buffer[offset + 3];
            if (!esByteContinuacionValido(b2) || !esByteContinuacionValido(b3) || !esByteContinuacionValido(b4)) {
                errores.push_back({offset, "Byte de continuación esperado, no encontrado."});
                offset += 1;
                continue;
            }
            uint32_t codePoint = (static_cast<uint32_t>(b1 & 0x07) << 18) |
                                  (static_cast<uint32_t>(b2 & 0x3F) << 12) |
                                  (static_cast<uint32_t>(b3 & 0x3F) << 6) |
                                  (static_cast<uint32_t>(b4 & 0x3F));

            // Bono: overlong encoding para secuencias de 4 bytes (mínimo válido: 0x10000)
            if (codePoint < 0x10000) {
                errores.push_back({offset, "Codificación sobrelarga (overlong encoding): el valor cabría en menos bytes."});
                offset += 4;
                continue;
            }

            codePoints.push_back({codePoint, 4});
            offset += 4;
            continue;
        }

        // Caso 5: byte de continuación huérfano (10xxxxxx sin líder previo) 
        if ((b1 & 0xC0) == 0x80) {
            errores.push_back({offset, "Byte de continuación inesperado sin byte líder previo."});
            offset += 1;
            continue;
        }

        // Caso 6: byte líder inválido (0xF8-0xFF, nunca válidos en UTF-8)
        errores.push_back({offset, "Byte líder inválido."});
        offset += 1;
    }
}


// Fase 4: Reporte carácter por carácter

void imprimirReporte(const std::vector<CodePointInfo>& codePoints) {
    std::cout << "=== Contenido decodificado ===\n";
    for (const auto& cp : codePoints) {
        if (cp.valor >= 0x20 && cp.valor <= 0x7E) {
            // Imprimible en ASCII: mostrar el carácter tal cual
            std::cout << static_cast<char>(cp.valor) << "\n";
        } else {
            // Todo lo demás: formato U+XXXX con std::format (obligatorio, sección 4.2.1)
            std::cout << std::format("U+{:04X}", cp.valor) << "\n";
        }
    }
}

void imprimirErrores(const std::vector<ErrorDecodificacion>& errores) {
    std::cout << "=== Errores detectados ===\n";
    for (const auto& err : errores) {
        std::cout << std::format("[offset {}] {}\n", err.offset, err.mensaje);
    }
}

// Fase 5: Resumen final 
void imprimirResumen(size_t bytesTotales,
                      const std::vector<CodePointInfo>& codePoints,
                      const std::vector<ErrorDecodificacion>& errores) {
    int conteo1 = 0, conteo2 = 0, conteo3 = 0, conteo4 = 0;
    for (const auto& cp : codePoints) {
        switch (cp.bytesUsados) {
            case 1: conteo1++; break;
            case 2: conteo2++; break;
            case 3: conteo3++; break;
            case 4: conteo4++; break;
        }
    }

    std::cout << "=== Resumen ===\n";
    std::cout << std::format("Bytes totales: {}\n", bytesTotales);
    std::cout << std::format("Code points válidos: {}\n", codePoints.size());
    std::cout << std::format(" - 1 byte: {}\n", conteo1);
    std::cout << std::format(" - 2 bytes: {}\n", conteo2);
    std::cout << std::format(" - 3 bytes: {}\n", conteo3);
    std::cout << std::format(" - 4 bytes: {}\n", conteo4);
    std::cout << std::format("Errores detectados: {}\n", errores.size());
}


int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Uso: " << argv[0] << " <archivo.txt>\n";
        return 1;
    }

    std::optional<std::vector<uint8_t>> resultado = leerArchivoBinario(argv[1]);

    if (!resultado.has_value()) {
        return 1;  // error real ya reportado dentro de leerArchivoBinario
    }

    std::vector<uint8_t> buffer = resultado.value();

    std::vector<CodePointInfo> codePoints;
    std::vector<ErrorDecodificacion> errores;

    decodificarUTF8(buffer, codePoints, errores);

    imprimirReporte(codePoints);
    imprimirErrores(errores);
    imprimirResumen(buffer.size(), codePoints, errores);

    return 0;
}