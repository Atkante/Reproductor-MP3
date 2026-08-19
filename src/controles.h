#include <Arduino.h>
#ifndef CONTROLES_H
#define CONTROLES_H
#define PIN_BTN_PREV 39
#define PIN_BTN_PLAY 40
#define PIN_BTN_NEXT 41

// Función para configurar los pines del encoder
void inicializarEncoder();

// Función que revisará si giraste la perilla o pulsaste el botón
void leerEncoder();

void inicializarBotonesMedia();
void leerBotonesMedia();
// Variables globales para compartir el estado del encoder con el main
extern int posicionEncoder;
extern bool botonPresionado;
extern bool botonPrevPresionado;
extern bool botonPlayPresionado;
extern bool botonNextPresionado;

#endif