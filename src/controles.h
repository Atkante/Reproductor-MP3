#ifndef CONTROLES_H
#define CONTROLES_H

#include <Arduino.h>

// Función para configurar los pines del encoder
void inicializarEncoder();

// Función que revisará si giraste la perilla o pulsaste el botón
void leerEncoder();

// Variables globales para compartir el estado del encoder con el main
extern int posicionEncoder;
extern bool botonPresionado;

#endif