#include <Arduino.h>
#include "ui.h"
#include "controles.h"
#include "almacenamiento.h"

enum EstadoSistema { MENU_PRINCIPAL, LISTA_CANCIONES, MODO_WIFI, MODO_BLUETOOTH };
EstadoSistema estadoActual = MENU_PRINCIPAL;

// Variables Menú Carrusel
int opcionMenu = 0;
const int TOTAL_OPCIONES = 3;
int opcionAnterior = -1;

// Variables Lista de Canciones
int indiceLista = 0;
int scrollLista = 0;
const int ITEMS_VISIBLES = 6;

bool sdLista = false;

void setup() {
    Serial.begin(115200);
    inicializarPantalla();
    inicializarEncoder();
    sdLista = inicializarSD();
    dibujarMenuPrincipal(opcionMenu);
}

void loop() {
    leerEncoder();

    // ==========================================
    // ESTADO 1: MENÚ PRINCIPAL (CARRUSEL)
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
            if (opcionMenu == 0) {
                // Entrar a la lista de reproducción
                estadoActual = LISTA_CANCIONES;
                if (sdLista) {
                    escanearCanciones();
                    indiceLista = 0; // Reiniciar cursor arriba
                    scrollLista = 0;
                    dibujarListaCanciones(indiceLista, scrollLista, listaCanciones);
                } else {
                    mostrarMensaje("Sin Tarjeta SD", TFT_RED);
                    delay(1500); // Mostramos el error 1.5s y volvemos
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
        
        // Si hay canciones y giramos el encoder...
        if (posicionEncoder != 0 && !listaCanciones.empty()) {
            indiceLista += posicionEncoder;
            posicionEncoder = 0;

            // Límites para no salirnos de la lista
            int total = listaCanciones.size();
            if (indiceLista < 0) indiceLista = 0;
            if (indiceLista >= total) indiceLista = total - 1;

            // Lógica de Desplazamiento (Scroll)
            // Si el cursor sube más allá de lo visible, arrastramos el scroll hacia arriba
            if (indiceLista < scrollLista) {
                scrollLista = indiceLista;
            }
            // Si el cursor baja más allá de lo visible, arrastramos el scroll hacia abajo
            if (indiceLista >= scrollLista + ITEMS_VISIBLES) {
                scrollLista = indiceLista - ITEMS_VISIBLES + 1;
            }

            // Redibujamos la lista actualizada
            dibujarListaCanciones(indiceLista, scrollLista, listaCanciones);
        }

        if (botonPresionado) {
            // Próximamente: Aquí conectaremos el Audio I2S para darle "Play".
            // Por ahora, al presionar regresamos al menú principal.
            estadoActual = MENU_PRINCIPAL;
            opcionAnterior = -1; // Forzamos a que el menú se dibuje limpio
        }
    }
    // ==========================================
    // OTROS ESTADOS (VOLVER AL MENÚ)
    // ==========================================
    else {
        if (botonPresionado) {
            estadoActual = MENU_PRINCIPAL;
            opcionAnterior = -1;
        }
    }
}