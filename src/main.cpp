#include <Arduino.h>
#include "ui.h"
#include "controles.h"
#include "almacenamiento.h"
#include "reproductor.h"

enum EstadoSistema { MENU_PRINCIPAL, LISTA_CANCIONES, VISTA_REPRODUCCION, MODO_WIFI, MODO_BLUETOOTH };
EstadoSistema estadoActual = MENU_PRINCIPAL;

int opcionMenu = 0;
const int TOTAL_OPCIONES = 3;
int opcionAnterior = -1;

int anguloDisco = 0;
unsigned long ultimoFrame = 0;

int indiceLista = 0;
int scrollLista = 0;
int indiceReproduciendo = -1; 
bool sdLista = false;

void setup() {
    Serial.begin(115200);
    inicializarPantalla();
    inicializarEncoder();
    sdLista = inicializarSD();
    inicializarAudio();
    dibujarMenuPrincipal(opcionMenu);
}

void loop() {
    procesarAudio(); 
    leerEncoder();

    // ==========================================
    // ESTADO 1: MENÚ PRINCIPAL
    // ==========================================
    if (estadoActual == MENU_PRINCIPAL) {
        if (posicionEncoder != 0) {
            opcionMenu += posicionEncoder;
            posicionEncoder = 0;
            if (opcionMenu < 0) opcionMenu = TOTAL_OPCIONES - 1;
            if (opcionMenu >= TOTAL_OPCIONES) opcionMenu = 0;
        }

        if (opcionMenu != opcionAnterior) {
            int direccion = (opcionMenu < opcionAnterior) ? -1 : 1; 
            if (opcionAnterior == 0 && opcionMenu == TOTAL_OPCIONES - 1) direccion = -1;
            if (opcionAnterior == TOTAL_OPCIONES - 1 && opcionMenu == 0) direccion = 1;

            if (opcionAnterior != -1) animarTransicion(opcionAnterior, opcionMenu, direccion);
            else dibujarMenuPrincipal(opcionMenu);
            
            opcionAnterior = opcionMenu;
        }

        if (botonPresionado) {
            botonPresionado = false; // <-- CONSUMIR CLIC

            if (opcionMenu == 0) {
                estadoActual = LISTA_CANCIONES;
                if (sdLista) {
                    escanearCanciones();
                    indiceLista = 0; scrollLista = 0;
                    dibujarListaCanciones(indiceLista, scrollLista, listaCanciones, true, indiceReproduciendo); 
                } else {
                    mostrarMensaje("Sin SD", TFT_RED);
                    delay(1500);
                    estadoActual = MENU_PRINCIPAL;
                    opcionAnterior = -1; 
                }
            } 
            else if (opcionMenu == 1) {
                estadoActual = MODO_WIFI;
                mostrarMensaje("Server WiFi...", TFT_BLUE);
            }
            else if (opcionMenu == 2) {
                estadoActual = MODO_BLUETOOTH;
                mostrarMensaje("Bluetooth...", TFT_DARKCYAN);
            }
        }
    } 
    // ==========================================
    // ESTADO 2: LISTA DE CANCIONES
    // ==========================================
    else if (estadoActual == LISTA_CANCIONES) {
        int totalItems = listaCanciones.size() + 1; 

        if (posicionEncoder != 0) {
            indiceLista += posicionEncoder;
            posicionEncoder = 0;
            if (indiceLista < 0) indiceLista = 0;
            if (indiceLista >= totalItems) indiceLista = totalItems - 1;

            if (indiceLista < scrollLista) scrollLista = indiceLista;
            if (indiceLista >= scrollLista + 6) scrollLista = indiceLista - 6 + 1;

            dibujarListaCanciones(indiceLista, scrollLista, listaCanciones, false, indiceReproduciendo); 
        }

        if (botonPresionado) {
            botonPresionado = false; // <-- CONSUMIR CLIC

            if (indiceLista == 0) {
                estadoActual = MENU_PRINCIPAL;
                opcionAnterior = -1; 
            } else {
                int indiceReal = indiceLista - 1;
                
                // 1. APAGAMOS LA LUZ DE FONDO
                animarFadeOutHW();
                
                // 2. PREPARAMOS GRÁFICOS (En la oscuridad)
                prepararVistaReproduccion(listaCanciones[indiceReal]);
                
                if (indiceReal != indiceReproduciendo) {
                    indiceReproduciendo = indiceReal;
                    reproducirCancion(listaCanciones[indiceReproduciendo]);
                }
                
                estadoActual = VISTA_REPRODUCCION;
                dibujarVolumen(volumenActual);
                // ---> AÑADE ESTA LÍNEA AQUÍ <---
                // Estampamos el disco en la oscuridad antes de encender la luz
                animarDiscoRotando(anguloDisco);
                // 3. ENCENDEMOS LA LUZ DE FONDO (Transición perfecta)
                animarFadeInHW();
            }
        }
    }
    // ==========================================
    // ESTADO 3: REPRODUCTOR 
    // ==========================================
    else if (estadoActual == VISTA_REPRODUCCION) {
        
        if (posicionEncoder != 0) {
            cambiarVolumen(posicionEncoder);
            posicionEncoder = 0;
            dibujarVolumen(volumenActual);
        }

        if (millis() - ultimoFrame > 40) {
            anguloDisco = (anguloDisco + 2) % 360;
            animarDiscoRotando(anguloDisco);
            ultimoFrame = millis();
        }

        if (cancionTerminada) {
            indiceReproduciendo++; 
            if (indiceReproduciendo >= listaCanciones.size()) indiceReproduciendo = 0; 
            
            // Aquí también podemos aplicar el Fade por hardware al cambiar de pista automáticamente
            animarFadeOutHW();
            prepararVistaReproduccion(listaCanciones[indiceReproduciendo]);
            reproducirCancion(listaCanciones[indiceReproduciendo]);
            dibujarVolumen(volumenActual);
            animarDiscoRotando(anguloDisco);
            animarFadeInHW();
        }

        if (botonPresionado) {
            botonPresionado = false; // <-- CONSUMIR CLIC
            
            // 1. APAGAMOS LA LUZ
            animarFadeOutHW();
            
            estadoActual = LISTA_CANCIONES;
            liberarDisco(); 
            
            // 2. DIBUJAMOS LA LISTA (En la oscuridad)
            dibujarListaCanciones(indiceLista, scrollLista, listaCanciones, true, indiceReproduciendo);
            
            // 3. ENCENDEMOS LA LUZ
            animarFadeInHW();
        }
    }
    else {
        if (botonPresionado) {
            botonPresionado = false; // <-- CONSUMIR CLIC
            estadoActual = MENU_PRINCIPAL;
            opcionAnterior = -1;
        }
    }
}