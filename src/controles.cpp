#include "controles.h"
#include <RotaryEncoder.h>

// --- ¡CAMBIA ESTOS PINES POR LOS TUYOS! ---
#define PIN_CLK 1  // Pin de reloj (A)
#define PIN_DT  2  // Pin de datos (B)
#define PIN_SW  42   // Pin del botón (Switch)

// Creamos el objeto del encoder
RotaryEncoder encoder(PIN_CLK, PIN_DT, RotaryEncoder::LatchMode::TWO03);

// Variables que le pasaremos al main
int posicionEncoder = 0;
bool botonPresionado = false;
int ultimaPosicion = 0;

void inicializarEncoder() {
    pinMode(PIN_SW, INPUT_PULLUP); // Configuramos el botón con resistencia interna
}

void leerEncoder() {
    // 1. Leemos la rotación
    encoder.tick();
    int nuevaPosicion = encoder.getPosition();
    
    if (nuevaPosicion != ultimaPosicion) {
        // Determinamos la dirección para sumar o restar a nuestra opción del menú
        if (nuevaPosicion > ultimaPosicion) {
            posicionEncoder++;
        } else {
            posicionEncoder--;
        }
        ultimaPosicion = nuevaPosicion;
    }

    // 2. Leemos el botón
    // El botón se lee como LOW cuando se presiona (por el INPUT_PULLUP)
    if (digitalRead(PIN_SW) == LOW) {
        botonPresionado = true;
        delay(200); // Pequeño antirrrebote simple para que no detecte 5 clicks seguidos
    } else {
        botonPresionado = false;
    }
}