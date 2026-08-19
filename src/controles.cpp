#include "controles.h"
#include <RotaryEncoder.h>

// --- ¡CAMBIA ESTOS PINES POR LOS TUYOS! ---
#define PIN_CLK 1  // Pin de reloj (A)
#define PIN_DT  2  // Pin de datos (B)
#define PIN_SW  42   // Pin del botón (Switch)

bool botonPrevPresionado = false;
bool botonPlayPresionado = false;
bool botonNextPresionado = false;

// Variables de estado y anti-rebote (Debounce)
bool estadoAntPrev = HIGH;
bool estadoAntPlay = HIGH;
bool estadoAntNext = HIGH;
unsigned long debouncePrev = 0;
unsigned long debouncePlay = 0;
unsigned long debounceNext = 0;

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
void inicializarBotonesMedia() {
    // Usamos resistencias pull-up internas. 
    // En la PCB solo debes conectar el botón entre el Pin y GND.
    pinMode(PIN_BTN_PREV, INPUT_PULLUP);
    pinMode(PIN_BTN_PLAY, INPUT_PULLUP);
    pinMode(PIN_BTN_NEXT, INPUT_PULLUP);
}

void leerBotonesMedia() {
    // Botón Anterior
    bool actPrev = digitalRead(PIN_BTN_PREV);
    if (actPrev == LOW && estadoAntPrev == HIGH && millis() - debouncePrev > 50) {
        botonPrevPresionado = true;
        debouncePrev = millis();
    }
    estadoAntPrev = actPrev;

    // Botón Play / Select
    bool actPlay = digitalRead(PIN_BTN_PLAY);
    if (actPlay == LOW && estadoAntPlay == HIGH && millis() - debouncePlay > 50) {
        botonPlayPresionado = true;
        debouncePlay = millis();
    }
    estadoAntPlay = actPlay;

    // Botón Siguiente
    bool actNext = digitalRead(PIN_BTN_NEXT);
    if (actNext == LOW && estadoAntNext == HIGH && millis() - debounceNext > 50) {
        botonNextPresionado = true;
        debounceNext = millis();
    }
    estadoAntNext = actNext;
}