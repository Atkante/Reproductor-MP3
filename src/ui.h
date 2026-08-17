#ifndef UI_H
#define UI_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <vector> // Necesario para recibir la lista

void inicializarPantalla();
void dibujarMenuPrincipal(int indiceSeleccionado, bool desdeAnimacion = false);
void dibujarReproductor(String nombreCancion, int progresoPorcentaje, bool reproduciendo);
void mostrarMensaje(String mensaje, uint16_t colorFondo);
void animarTransicion(int indiceViejo, int indiceNuevo, int direccion);

// NUEVA FUNCIÓN: Dibuja la lista de canciones interactiva
void dibujarListaCanciones(int indiceSeleccionado, int scrollOffset, const std::vector<String>& lista);

#endif