#include "almacenamiento.h"
#include <SPI.h>
#include <SD.h>
#include <FS.h>

// Pines de tu placa
#define SD_SCK   15
#define SD_MISO   6
#define SD_MOSI   7
#define SD_CS     5

// Volvemos a usar HSPI como en tu código funcional
SPIClass sdSPI(HSPI);

// Vector donde almacenaremos las canciones encontradas
std::vector<String> listaCanciones;

bool inicializarSD() {
    // Inicializamos el bus SPI dedicado
    sdSPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);

    // Tu lógica de arranque resistente a fallos: hasta 8 intentos
    for (int i = 1; i <= 8; i++) {
        // Velocidad a 4MHz garantizada para cables de prueba
        if (SD.begin(SD_CS, sdSPI, 4000000)) {
            Serial.printf("[SD] Tarjeta montada correctamente en el intento %d\n", i);
            
            // Verificación extra de seguridad
            uint8_t tipo = SD.cardType();
            if (tipo == CARD_NONE) {
                Serial.println("[SD] Error: Se conecto pero no hay tarjeta física insertada.");
                return false;
            }
            return true;
        }
        
        Serial.printf("[SD] Intento %d fallido, reintentando...\n", i);
        SD.end(); // Liberamos el bus antes de reintentar
        delay(300); // Le damos tiempo al condensador del modulo SD para cargar
    }

    Serial.println("[SD] Error definitivo: No se pudo montar la tarjeta despues de 8 intentos.");
    return false;
}

void escanearCanciones() {
    listaCanciones.clear();

    File raiz = SD.open("/");
    if (!raiz || !raiz.isDirectory()) {
        Serial.println("[SD] Error al abrir el directorio raiz.");
        return;
    }

    File archivo = raiz.openNextFile();
    while (archivo) {
        if (!archivo.isDirectory()) {
            String nombre = String(archivo.name());
            
            // Filtramos archivos MP3 (mayúsculas o minúsculas)
            if (nombre.endsWith(".mp3") || nombre.endsWith(".MP3")) {
                if (nombre.startsWith("/")) {
                    nombre = nombre.substring(1);
                }
                listaCanciones.push_back(nombre);
                Serial.printf("[SD] Encontrada: %s\n", nombre.c_str());
            }
        }
        archivo = raiz.openNextFile();
    }

    Serial.printf("[SD] Total de canciones cargadas: %d\n", listaCanciones.size());
}