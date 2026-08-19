#include <Arduino.h>
#include <WiFi.h> // Necesario para obtener la IP y mostrarla
#include "ui.h"
#include "controles.h"
#include "almacenamiento.h"
#include "reproductor.h"
#include "servidor.h" // <-- Incluimos nuestro nuevo módulo

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
bool estaPausado = false;
void setup() {
    Serial.begin(115200);
    inicializarPantalla();
    inicializarEncoder();
    sdLista = inicializarSD();
    inicializarAudio();
    
    // NOTA: NO iniciamos el WiFi aquí para ahorrar batería y evitar ruidos en el audio.
    
    dibujarMenuPrincipal(opcionMenu);
    inicializarEncoder();
    inicializarBotonesMedia();
}

void loop() {
    procesarAudio(); 
    leerEncoder();
    leerBotonesMedia();
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

        if (botonPresionado || botonPlayPresionado) {
            botonPresionado = false; // <-- CONSUMIR CLIC
            botonPlayPresionado = false;
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
                // Preparamos la pantalla para cargar el servidor
                estadoActual = MODO_WIFI;
                mostrarMensaje("Iniciando WiFi...", TFT_BLUE);
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

        if (botonPresionado || botonPlayPresionado) {
            botonPresionado = false; // <-- CONSUMIR CLIC
            botonPlayPresionado = false;
            if (indiceLista == 0) {
                estadoActual = MENU_PRINCIPAL;
                opcionAnterior = -1; 
            } else {
                int indiceReal = indiceLista - 1;
                
                animarFadeOutHW();
                prepararVistaReproduccion(listaCanciones[indiceReal]);
                
                if (indiceReal != indiceReproduciendo) {
                    indiceReproduciendo = indiceReal;
                    reproducirCancion(listaCanciones[indiceReproduciendo]);
                }
                
                estadoActual = VISTA_REPRODUCCION;
                dibujarVolumen(volumenActual);
                animarDiscoRotando(anguloDisco); 
                animarFadeInHW();
            }
        }
    }
    // ==========================================
    // ESTADO 3: REPRODUCTOR 
    // ==========================================
    else if (estadoActual == VISTA_REPRODUCCION) {
        
        // Volumen con Encoder
        if (posicionEncoder != 0) {
            cambiarVolumen(posicionEncoder);
            posicionEncoder = 0;
            dibujarVolumen(volumenActual);
        }

        // Animación del disco SÓLO si no está pausado
        if (!estaPausado && millis() - ultimoFrame > 40) {
            anguloDisco = (anguloDisco + 2) % 360;
            animarDiscoRotando(anguloDisco);
            ultimoFrame = millis();
        }

        // --- BOTÓN PLAY / PAUSE ---
        if (botonPlayPresionado) {
            botonPlayPresionado = false;
            estaPausado = !estaPausado;
            pausarReanudarCancion(); // Detiene o arranca la música
        }

        // --- BOTONES SIGUIENTE Y ANTERIOR ---
        if (botonNextPresionado || botonPrevPresionado || cancionTerminada) {
            
            if (botonNextPresionado || cancionTerminada) {
                indiceReproduciendo++; 
                if (indiceReproduciendo >= listaCanciones.size()) indiceReproduciendo = 0; 
            } 
            else if (botonPrevPresionado) {
                indiceReproduciendo--;
                if (indiceReproduciendo < 0) indiceReproduciendo = listaCanciones.size() - 1;
            }

            botonNextPresionado = false;
            botonPrevPresionado = false;
            
            animarFadeOutHW();
            prepararVistaReproduccion(listaCanciones[indiceReproduciendo]);
            reproducirCancion(listaCanciones[indiceReproduciendo]);
            dibujarVolumen(volumenActual);
            
            estaPausado = false; // Al cambiar de canción quitamos la pausa
            animarDiscoRotando(anguloDisco); 
            animarFadeInHW();
        }

        // --- BOTÓN DEL ENCODER (Volver a la lista) ---
        if (botonPresionado || botonPlayPresionado) {
            botonPresionado = false;     // <-- Consumir ambos clics
            botonPlayPresionado = false;
            animarFadeOutHW();
            estadoActual = LISTA_CANCIONES;
            liberarDisco(); 
            dibujarListaCanciones(indiceLista, scrollLista, listaCanciones, true, indiceReproduciendo);
            animarFadeInHW();
        }
    }
    // ==========================================
    // ESTADO 4: MODO WIFI (On-Demand)
    // ==========================================
    else if (estadoActual == MODO_WIFI) {
        static bool wifiIniciado = false;

        // 1. Encender y levantar el servidor SÓLO al entrar
        if (!wifiIniciado) {
            inicializarSistemaWiFi();
            
            String ipActual = (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA) ? 
                              WiFi.softAPIP().toString() : WiFi.localIP().toString();
                              
            mostrarMensaje("Web: " + ipActual, TFT_BLUE);
            wifiIniciado = true;
        }

        // 2. Apagar y salir cuando el usuario presione el botón
        if (botonPresionado || botonPlayPresionado){
            botonPresionado = false; // <-- CONSUMIR CLIC
            botonPlayPresionado = false;            
            mostrarMensaje("Apagando...", TFT_DARKGREY);
            detenerWiFi(); // Destruimos el servidor y apagamos antena
            wifiIniciado = false;
            delay(500); // Pequeña pausa para que se limpie la memoria de radio
            
            estadoActual = MENU_PRINCIPAL;
            opcionAnterior = -1;
            dibujarMenuPrincipal(opcionMenu);
        }
    }
    // ==========================================
    // ESTADO 5: MODO BLUETOOTH (Futuro)
    // ==========================================
    else {
        if (botonPresionado || botonPlayPresionado) {
            botonPresionado = false; 
            botonPlayPresionado = false;
            estadoActual = MENU_PRINCIPAL;
            opcionAnterior = -1;
            dibujarMenuPrincipal(opcionMenu);
        }
    }
}