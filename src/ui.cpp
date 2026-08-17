#include "ui.h"
#include <SPI.h>

// MANTÉN ESTA LÍNEA EXACTAMENTE COMO LA TENÍAS
TFT_eSPI tft = TFT_eSPI(); 
TFT_eSprite spriteTransicion = TFT_eSprite(&tft);
const char* nombresMenu[] = {"Reproducir cancion", "Modo WiFi", "Bluetooth"};
const int TOTAL_OPCIONES = 3;

void inicializarPantalla() {
    tft.init();
    tft.setRotation(1); 
    tft.fillScreen(TFT_BLACK);
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
void dibujarListaCanciones(int indiceSeleccionado, int scrollOffset, const std::vector<String>& lista) {
    tft.fillScreen(TFT_BLACK);

    // --- CABECERA ---
    tft.fillRect(0, 0, tft.width(), 30, tft.color565(255, 100, 0)); // Naranja
    tft.setTextColor(TFT_WHITE, tft.color565(255, 100, 0));
    tft.setTextDatum(MC_DATUM);
    tft.drawString("LISTA DE REPRODUCCION", tft.width() / 2, 15, 2);

    if (lista.empty()) {
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.drawString("No hay archivos .mp3", tft.width() / 2, tft.height() / 2, 2);
        return;
    }

    // --- LISTA DESLIZABLE ---
    int yInicio = 45;
    int altoFila = 30;
    int itemsVisibles = 6; // Cantidad de canciones que caben en la pantalla
    
    tft.setTextDatum(ML_DATUM); // Alinear a la izquierda (Medio)

    for (int i = scrollOffset; i < lista.size() && i < scrollOffset + itemsVisibles; i++) {
        int y = yInicio + ((i - scrollOffset) * altoFila);
        
        // Si es la canción seleccionada, pintamos el fondo de resalte
        if (i == indiceSeleccionado) {
            tft.fillRect(0, y - 15, tft.width(), altoFila, tft.color565(50, 50, 50)); // Gris oscuro
            tft.setTextColor(tft.color565(255, 100, 0), tft.color565(50, 50, 50)); // Texto Naranja
        } else {
            tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
        }

        // Cortamos el nombre si es muy largo para que no se desborde
        String nombre = lista[i];
        if (nombre.length() > 22) nombre = nombre.substring(0, 20) + "..";
        
        // Añadimos un cursor ">" a la opción activa
        tft.drawString((i == indiceSeleccionado ? "> " : "  ") + nombre, 10, y, 2);
    }
}