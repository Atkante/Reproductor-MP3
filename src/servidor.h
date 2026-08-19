#ifndef SERVIDOR_H
#define SERVIDOR_H

#include <Arduino.h>

void inicializarSistemaWiFi();
void procesarServidor();
bool estaEnModoConfiguracion();
void detenerWiFi();
#endif