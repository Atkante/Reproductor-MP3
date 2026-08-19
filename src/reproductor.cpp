#include "reproductor.h"
#include <Audio.h>
#include <SD.h>

Audio audio;
bool cancionTerminada = false;
int volumenActual = 12;

#define I2S_BCLK  16
#define I2S_LRC   17
#define I2S_DOUT  18

void inicializarAudio() {
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(volumenActual);
}

void reproducirCancion(String nombreMp3) {
    cancionTerminada = false;
    String ruta = "/" + nombreMp3;
    audio.connecttoFS(SD, ruta.c_str());
}

void detenerAudio() {
    audio.stopSong();
}

void procesarAudio() {
    audio.loop(); 
}

void cambiarVolumen(int delta) {
    volumenActual += delta;
    if (volumenActual < 0) volumenActual = 0;
    if (volumenActual > 21) volumenActual = 21; // Max volumen de la libreria
    audio.setVolume(volumenActual);
}

void audio_eof_mp3(const char *info) {
    cancionTerminada = true;
}
void pausarReanudarCancion() {
    audio.pauseResume(); // Esta función nativa intercala entre pausa y play
}