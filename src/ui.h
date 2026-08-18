#ifndef UI_H
#define UI_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <vector>

void inicializarPantalla();
void dibujarMenuPrincipal(int indiceSeleccionado, bool desdeAnimacion = false);
void mostrarMensaje(String mensaje, uint16_t colorFondo);
void animarTransicion(int indiceViejo, int indiceNuevo, int direccion);

void dibujarListaCanciones(int indiceSeleccionado, int scrollOffset, const std::vector<String>& lista, bool redibujarFondo, int indiceSonando);

void prepararVistaReproduccion(String nombreMp3);
void animarDiscoRotando(int angulo);
void dibujarVolumen(int volumen);

// --- NUEVO EFECTO DE TRANSICIÓN ---
void animarFadeOut();
void animarFadeOutHW();
void animarFadeInHW();

// --- EXPOSICIÓN SEGURA PARA EL MAIN ---
extern bool hayPortada;
void liberarDisco();

#endif