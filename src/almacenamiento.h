#ifndef ALMACENAMIENTO_H
#define ALMACENAMIENTO_H

#include <Arduino.h>
#include <vector>

// Función para montar e inicializar la tarjeta SD
bool inicializarSD();

// Función para buscar todos los archivos .mp3 en la raíz de la SD
void escanearCanciones();

// Lista dinámica global con los nombres de los archivos encontrados
extern std::vector<String> listaCanciones;

#endif