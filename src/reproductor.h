#ifndef REPRODUCTOR_H
#define REPRODUCTOR_H

#include <Arduino.h>

void inicializarAudio();
void reproducirCancion(String nombreMp3);
void detenerAudio();
void procesarAudio();
void cambiarVolumen(int delta);

extern bool cancionTerminada;
extern int volumenActual;

#endif