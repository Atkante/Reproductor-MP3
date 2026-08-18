#include "ui.h"
#include <SPI.h>
#include <TJpg_Decoder.h>
#include <SD.h>
// --- CONFIGURACIÓN DE LA LUZ DE FONDO ---
#define TFT_BL 3        // El pin donde conectaste el BL de la pantalla
#define CANAL_PWM 0      // Canal interno del ESP32 para controlar la luz
// MANTÉN ESTA LÍNEA EXACTAMENTE COMO LA TENÍAS
TFT_eSPI tft = TFT_eSPI(); 
TFT_eSprite spriteTransicion = TFT_eSprite(&tft);
const char* nombresMenu[] = {"Reproducir cancion", "Modo WiFi", "Bluetooth"};
const int TOTAL_OPCIONES = 3;
TFT_eSprite discoVirtual = TFT_eSprite(&tft);
bool hayPortada = false;

void inicializarPantalla() {
    // 1. Configuramos el hardware atenuador de luz del ESP32
    ledcSetup(CANAL_PWM, 5000, 8); // 5000 Hz de frecuencia, 8 bits de resolución (0-255)
    ledcAttachPin(TFT_BL, CANAL_PWM);
    
    // 2. Encendemos la luz al 100% de brillo (255)
    ledcWrite(CANAL_PWM, 255); 

    tft.init();
    tft.setRotation(1); 
    tft.fillScreen(TFT_BLACK);
}
void animarFadeOutHW() {
    // Bajamos la luz de 255 a 0 gradualmente
    for (int brillo = 255; brillo >= 0; brillo -= 5) {
        ledcWrite(CANAL_PWM, brillo);
        delay(2); // Ajusta este delay si quieres que el apagado sea más rápido o lento
    }
    ledcWrite(CANAL_PWM, 0); // Aseguramos apagado total
}

void animarFadeInHW() {
    // Subimos la luz de 0 a 255 gradualmente
    for (int brillo = 0; brillo <= 255; brillo += 5) {
        ledcWrite(CANAL_PWM, brillo);
        delay(2); // El encendido suele verse mejor un poco más lento
    }
    ledcWrite(CANAL_PWM, 255); // Aseguramos brillo máximo
}

void dibujarMenuPrincipal(int indice, bool desdeAnimacion) {
    int centroX = tft.width() / 2;
    int centroY = tft.height() / 2 - 20;

    // Solo borramos la pantalla y dibujamos círculos SI NO venimos de animar
    if (!desdeAnimacion) {
        tft.fillScreen(TFT_BLACK);

        // Círculos grises de los lados
        tft.fillCircle(centroX - 80, centroY, 15, tft.color565(40, 40, 40)); 
        tft.fillCircle(centroX + 80, centroY, 15, tft.color565(40, 40, 40));

        // Icono Central
        int radioActivo = 45;
        if (indice == 0) {
            tft.fillCircle(centroX, centroY, radioActivo, tft.color565(255, 100, 0));
            tft.fillTriangle(centroX - 12, centroY - 18, centroX - 12, centroY + 18, centroX + 18, centroY, TFT_WHITE);
        } else if (indice == 1) {
            tft.fillCircle(centroX, centroY, radioActivo, TFT_BLUE);
            tft.fillRect(centroX - 15, centroY - 15, 30, 6, TFT_WHITE);
            tft.fillRect(centroX - 10, centroY - 2, 20, 6, TFT_WHITE);
            tft.fillRect(centroX - 5, centroY + 11, 10, 6, TFT_WHITE);
        } else if (indice == 2) {
            tft.fillCircle(centroX, centroY, radioActivo, TFT_DARKCYAN);
            tft.drawLine(centroX, centroY - 20, centroX, centroY + 20, TFT_WHITE); 
            tft.drawLine(centroX, centroY - 20, centroX + 15, centroY - 5, TFT_WHITE);
            tft.drawLine(centroX + 15, centroY - 5, centroX - 15, centroY + 10, TFT_WHITE);
            tft.drawLine(centroX, centroY + 20, centroX + 15, centroY + 5, TFT_WHITE);
            tft.drawLine(centroX + 15, centroY + 5, centroX - 15, centroY - 10, TFT_WHITE);
        }
    }

    // --- TEXTOS --- (Esto siempre se actualiza)
    // tft.setTextColor con dos colores asegura que el fondo del texto sea sólido
    tft.setTextColor(TFT_WHITE, TFT_BLACK); 
    tft.setTextDatum(BC_DATUM);
    tft.drawString(nombresMenu[indice], centroX, centroY + 80, 4); 
    
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft.drawString("< Gira la perilla >", centroX, tft.height() - 10, 2);
}

void animarTransicion(int indiceViejo, int indiceNuevo, int direccion) {
    int centroX = tft.width() / 2;
    int centroY = tft.height() / 2 - 20;
    
    // 1. ¡CERO fillScreen! Solo pintamos un cuadro negro justo donde estaba el texto viejo
    tft.fillRect(0, centroY + 50, tft.width(), 80, TFT_BLACK);

    spriteTransicion.setColorDepth(16);
    spriteTransicion.createSprite(tft.width(), 110); 
    spriteTransicion.setSwapBytes(true);

    int totalFrames = 6;      
    int distanciaViaje = 180; 

    for(int frame = 1; frame <= totalFrames; frame++) {
        spriteTransicion.fillSprite(TFT_BLACK); 
        
        spriteTransicion.fillCircle(centroX - 80, 55, 15, tft.color565(40, 40, 40)); 
        spriteTransicion.fillCircle(centroX + 80, 55, 15, tft.color565(40, 40, 40));

        int desplazamientoViejo = direccion * (distanciaViaje * frame / totalFrames); 
        int desplazamientoNuevo = direccion * (-distanciaViaje + (distanciaViaje * frame / totalFrames));

        int xV = centroX + desplazamientoViejo;
        if (indiceViejo == 0) {
            spriteTransicion.fillCircle(xV, 55, 45, tft.color565(255, 100, 0));
            spriteTransicion.fillTriangle(xV - 12, 55 - 18, xV - 12, 55 + 18, xV + 18, 55, TFT_WHITE);
        } else if (indiceViejo == 1) {
            spriteTransicion.fillCircle(xV, 55, 45, TFT_BLUE);
            spriteTransicion.fillRect(xV - 15, 55 - 15, 30, 6, TFT_WHITE);
            spriteTransicion.fillRect(xV - 10, 55 - 2, 20, 6, TFT_WHITE);
            spriteTransicion.fillRect(xV - 5, 55 + 11, 10, 6, TFT_WHITE);
        } else if (indiceViejo == 2) {
            spriteTransicion.fillCircle(xV, 55, 45, TFT_DARKCYAN);
            spriteTransicion.drawLine(xV, 55 - 20, xV, 55 + 20, TFT_WHITE); 
            spriteTransicion.drawLine(xV, 55 - 20, xV + 15, 55 - 5, TFT_WHITE);
            spriteTransicion.drawLine(xV + 15, 55 - 5, xV - 15, 55 + 10, TFT_WHITE);
            spriteTransicion.drawLine(xV, 55 + 20, xV + 15, 55 + 5, TFT_WHITE);
            spriteTransicion.drawLine(xV + 15, 55 + 5, xV - 15, 55 - 10, TFT_WHITE);
        }

        int xN = centroX + desplazamientoNuevo;
        if (indiceNuevo == 0) {
            spriteTransicion.fillCircle(xN, 55, 45, tft.color565(255, 100, 0));
            spriteTransicion.fillTriangle(xN - 12, 55 - 18, xN - 12, 55 + 18, xN + 18, 55, TFT_WHITE);
        } else if (indiceNuevo == 1) {
            spriteTransicion.fillCircle(xN, 55, 45, TFT_BLUE);
            spriteTransicion.fillRect(xN - 15, 55 - 15, 30, 6, TFT_WHITE);
            spriteTransicion.fillRect(xN - 10, 55 - 2, 20, 6, TFT_WHITE);
            spriteTransicion.fillRect(xN - 5, 55 + 11, 10, 6, TFT_WHITE);
        } else if (indiceNuevo == 2) {
            spriteTransicion.fillCircle(xN, 55, 45, TFT_DARKCYAN);
            spriteTransicion.drawLine(xN, 55 - 20, xN, 55 + 20, TFT_WHITE); 
            spriteTransicion.drawLine(xN, 55 - 20, xN + 15, 55 - 5, TFT_WHITE);
            spriteTransicion.drawLine(xN + 15, 55 - 5, xN - 15, 55 + 10, TFT_WHITE);
            spriteTransicion.drawLine(xN, 55 + 20, xN + 15, 55 + 5, TFT_WHITE);
            spriteTransicion.drawLine(xN + 15, 55 + 5, xN - 15, 55 - 10, TFT_WHITE);
        }

        spriteTransicion.pushSprite(0, centroY - 55); 
    }
    
    spriteTransicion.deleteSprite();
    
    // 2. Avisamos que venimos de la animación (true)
    dibujarMenuPrincipal(indiceNuevo, true);
}
void mostrarMensaje(String mensaje, uint16_t colorFondo) {
    tft.fillScreen(colorFondo);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_WHITE, colorFondo); 
    tft.drawString(mensaje, tft.width() / 2, tft.height() / 2, 4); 
}

// Por si acaso también borraste la del reproductor en los reemplazos, asegúrate de tener esta también:
void dibujarReproductor(String nombreCancion, int progresoPorcentaje, bool reproduciendo) {
    tft.fillScreen(TFT_NAVY);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_CYAN, TFT_NAVY);
    tft.drawString(nombreCancion, tft.width() / 2, 60, 4);
}


void dibujarListaCanciones(int indiceSeleccionado, int scrollOffset, const std::vector<String>& lista, bool redibujarFondo, int indiceSonando) {
    if (redibujarFondo) {
        tft.fillScreen(TFT_BLACK);
        tft.fillRect(0, 0, tft.width(), 30, tft.color565(255, 100, 0)); 
        tft.setTextColor(TFT_WHITE, tft.color565(255, 100, 0));
        tft.setTextDatum(MC_DATUM);
        tft.drawString("LISTA DE REPRODUCCION", tft.width() / 2, 15, 2);
        
        if (indiceSonando != -1) {
            tft.setTextColor(TFT_GREEN, TFT_BLACK);
            tft.drawString("Audio en 2do plano...", tft.width() / 2, tft.height() - 10, 1);
        }
    }

    int yInicio = 35;
    int altoFila = 30;
    tft.setTextDatum(ML_DATUM);

    // Iteramos +1 porque el elemento 0 será "Volver al Menú"
    int totalItems = lista.size() + 1; 

    for (int i = scrollOffset; i < totalItems && i < scrollOffset + 6; i++) {
        int y = yInicio + ((i - scrollOffset) * altoFila);
        
        uint16_t colorFondo = (i == indiceSeleccionado) ? tft.color565(50, 50, 50) : TFT_BLACK;
        uint16_t colorTexto = (i == indiceSeleccionado) ? tft.color565(255, 100, 0) : TFT_LIGHTGREY;
        
        tft.fillRect(0, y, tft.width(), altoFila, colorFondo);
        tft.setTextColor(colorTexto, colorFondo);

        String texto = "";
        if (i == 0) {
            texto = "< Volver al Menu >";
        } else {
            texto = lista[i - 1]; // -1 porque el 0 es volver
            if (texto.length() > 22) texto = texto.substring(0, 20) + "..";
            // Resaltar en verde la canción que está sonando actualmente
            if (i - 1 == indiceSonando && i != indiceSeleccionado) {
                tft.setTextColor(TFT_GREEN, colorFondo);
            }
        }
        
        tft.drawString((i == indiceSeleccionado ? "> " : "  ") + texto, 10, y + 15, 2);
    }
}
// --- CALLBACK 1: FONDO OSCURECIDO ---
bool tft_output_bg(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
    for(int i = 0; i < w * h; i++) {
        uint16_t c = bitmap[i];
        
        // Matemática limpia con colores sin invertir
        uint16_t r = ((c >> 11) & 0x1F) / 3;
        uint16_t g = ((c >> 5) & 0x3F) / 3;
        uint16_t b = (c & 0x1F) / 3;
        bitmap[i] = (r << 11) | (g << 5) | b; 
    }
    
    // Invertimos los bytes solo en la pantalla justo antes de pintar
    bool oldSwap = tft.getSwapBytes();
    tft.setSwapBytes(true);
    tft.pushImage(x, y, w, h, bitmap);
    tft.setSwapBytes(oldSwap);
    
    return 1;
}

// --- CALLBACK 2: SPRITE RECORTADO 200x200 ---
bool tft_output_sprite(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
    if (x >= 220 || y >= 220 || x + w <= 20 || y + h <= 20) return 1; 

    for (int row = 0; row < h; row++) {
        int destY = (y + row) - 20; 
        if (destY < 0 || destY >= 200) continue;

        for (int col = 0; col < w; col++) {
            int destX = (x + col) - 20;
            if (destX < 0 || destX >= 200) continue;
            
            // drawPixel usa colores nativos sin invertir, así que ahora funcionará perfecto
            discoVirtual.drawPixel(destX, destY, bitmap[row * w + col]);
        }
    }
    return 1;
}

void prepararVistaReproduccion(String nombreMp3) {
    hayPortada = false;
    String archivoImg = "/" + nombreMp3;
    archivoImg.replace(".mp3", ".jpg");
    archivoImg.replace(".MP3", ".jpg");

    if (!SD.exists(archivoImg)) {
        tft.fillScreen(TFT_NAVY);
        tft.drawCircle(tft.width()/2, tft.height()/2, 90, TFT_WHITE);
        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(TFT_WHITE, TFT_NAVY);
        tft.drawString("Sin Portada", tft.width()/2, tft.height()/2, 2);
        return;
    }

    // 1. DIBUJAR FONDO OSCURO A PANTALLA COMPLETA
    TJpgDec.setJpgScale(1); 
    
    // --- LA MAGIA: Falso para no dañar las matemáticas de color ---
    TJpgDec.setSwapBytes(false); 
    
    TJpgDec.setCallback(tft_output_bg);
    TJpgDec.drawSdJpg(0, 0, archivoImg.c_str());

    // Dibujamos un "plato" negro para que las esquinas rotadas no dejen rastro
    tft.fillCircle(tft.width()/2, tft.height()/2, 100, TFT_BLACK); 

    // 2. CREAR DISCO GRANDE (200x200)
    if (!discoVirtual.created()) {
        discoVirtual.createSprite(200, 200);
        discoVirtual.setSwapBytes(true); // El lienzo debe invertir los colores al enviarlos a la pantalla
    }
    
    discoVirtual.fillSprite(TFT_MAGENTA);
    
    TJpgDec.setCallback(tft_output_sprite);
    TJpgDec.drawSdJpg(0, 0, archivoImg.c_str());

    // Hacerlo redondo matemáticamente
    int r = 100; // Radio
    for (int x = 0; x < 200; x++) {
        for (int y = 0; y < 200; y++) {
            if ((x-100)*(x-100) + (y-100)*(y-100) > r*r) {
                discoVirtual.drawPixel(x, y, TFT_MAGENTA); 
            }
        }
    }
    
    discoVirtual.setPivot(100, 100); 
    tft.setPivot(tft.width()/2, tft.height()/2);
    hayPortada = true;
}
void dibujarVolumen(int volumen) {
    tft.fillRect(0, tft.height() - 25, tft.width(), 25, tft.color565(30, 30, 30));
    tft.setTextColor(TFT_WHITE, tft.color565(30, 30, 30));
    tft.setTextDatum(MC_DATUM);
    tft.drawString("Volumen: " + String(volumen) + " / 21", tft.width()/2, tft.height() - 12, 2);
}

void liberarDisco() {
    if (discoVirtual.created()) {
        discoVirtual.deleteSprite();
    }
}
void animarDiscoRotando(int angulo) {
    if (hayPortada) {
        discoVirtual.pushRotated(angulo, TFT_MAGENTA);
    }
}
// Añade esto al final de ui.cpp
void animarFadeOut() {
int centroX = tft.width() / 2;
    int centroY = tft.height() / 2;
    int radioMax = 175; // Matemáticamente, 175px de radio cubre las esquinas de una pantalla de 240x240

    int velocidad = 2; // Inicia suave
    
    // El círculo oscuro crece desde el centro empujando hacia afuera
    for (int r = 0; r <= radioMax; r += velocidad) {
        tft.fillCircle(centroX, centroY, r, TFT_BLACK);
        
        velocidad += 2; // Aceleración matemática (curva Ease-In)
        delay(5);       // Pausa microscópica para la fluidez
    }
    
    // Aseguramos que la pantalla quede limpia
    tft.fillScreen(TFT_BLACK);
    delay(20);
}