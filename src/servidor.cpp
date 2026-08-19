#include "servidor.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include <SD.h>
#include <Update.h>

AsyncWebServer servidor(80);
Preferences preferences;
bool modoAPActivo = false;
static bool rutasConfiguradas = false; 

// HTML con la Inteligencia de Procesamiento de Imágenes (JavaScript)
const char paginaHTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Reproductor MP3 - Gestor</title>
  <style>
    body { font-family: Arial, sans-serif; background: #121212; color: #fff; text-align: center; padding: 15px; }
    .card { background: #1e1e1e; padding: 15px; border-radius: 10px; margin: 15px auto; max-width: 400px; box-shadow: 0 4px 10px rgba(0,0,0,0.5); }
    input[type="file"], input[type="text"], input[type="password"] { width: 90%; padding: 10px; margin: 8px 0; background: #2c2c2c; border: 1px solid #444; color: #fff; border-radius: 5px; }
    button { background: #ff6400; color: white; border: none; padding: 12px 20px; font-weight: bold; border-radius: 5px; cursor: pointer; width: 100%; margin-top: 10px;}
    button:hover { background: #e05b00; }
    h2 { color: #ff6400; font-size: 22px; }
    h3 { font-size: 16px; border-bottom: 1px solid #333; padding-bottom: 5px; }
    #status { margin-top: 10px; font-weight: bold; color: #4CAF50; }
  </style>
</head>
<body>
  <h2>🎵 Reproductor MP3 ESP32</h2>
  
  <div class="card">
    <h3>Subir Nueva Canción y Portada</h3>
    <form id="uploadForm">
      <label style="display:block; text-align:left; margin-left:5%;">1. Archivo de Audio (.mp3)</label>
      <input type="file" id="mp3File" accept=".mp3" required><br><br>
      
      <label style="display:block; text-align:left; margin-left:5%;">2. Portada (Opcional - Se ajustará a 240x240)</label>
      <input type="file" id="coverFile" accept="image/*"><br>
      
      <button type="submit" id="btnSubir">Procesar y Subir a la SD</button>
      <div id="status"></div>
    </form>
  </div>

  <div class="card">
    <h3>Configurar Red WiFi</h3>
    <form action="/guardar-wifi" method="POST">
      <input type="text" name="ssid" placeholder="Nombre de tu Red WiFi" required><br>
      <input type="password" name="password" placeholder="Contraseña WiFi"><br>
      <button type="submit">Guardar y Conectar</button>
    </form>
  </div>

  <div class="card">
    <h3>Actualizar Firmware (OTA)</h3>
    <form action="/update" method="POST" enctype="multipart/form-data">
      <input type="file" name="update" required><br>
      <button type="submit" style="background: #28a745;">Actualizar Sistema</button>
    </form>
  </div>

  <script>
    document.getElementById('uploadForm').onsubmit = function(e) {
      e.preventDefault();
      let btn = document.getElementById('btnSubir');
      let status = document.getElementById('status');
      
      btn.disabled = true;
      status.style.color = "#ffcc00";
      status.innerText = "Procesando archivos...";

      let mp3Input = document.getElementById('mp3File');
      let coverInput = document.getElementById('coverFile');
      let formData = new FormData();

      if (mp3Input.files.length === 0) return;
      let mp3File = mp3Input.files[0];
      
      // Extraemos el nombre sin el .mp3 para usarlo en la imagen
      let baseName = mp3File.name.substring(0, mp3File.name.lastIndexOf('.'));
      formData.append('file1', mp3File, mp3File.name);

      if (coverInput.files.length > 0) {
        let coverFile = coverInput.files[0];
        let img = new Image();
        let objUrl = URL.createObjectURL(coverFile);
        
        img.onload = function() {
          // Creamos un lienzo de 240x240
          let canvas = document.createElement('canvas');
          canvas.width = 240; canvas.height = 240;
          let ctx = canvas.getContext('2d');
          
          // Recortamos el centro de la imagen para que quede cuadrada perfecta
          let size = Math.min(img.width, img.height);
          let sx = (img.width - size) / 2;
          let sy = (img.height - size) / 2;
          ctx.drawImage(img, sx, sy, size, size, 0, 0, 240, 240);

          // Convertimos el lienzo a un JPG super ligero
          canvas.toBlob(function(blob) {
            // Le ponemos el mismo nombre que al MP3
            formData.append('file2', blob, baseName + '.jpg');
            enviarDatos(formData, btn, status);
            URL.revokeObjectURL(objUrl);
          }, 'image/jpeg', 0.85); // 85% de calidad es perfecto
        };
        img.src = objUrl;
      } else {
        enviarDatos(formData, btn, status);
      }
    };

    function enviarDatos(formData, btn, status) {
      status.style.color = "#00aaff";
      status.innerText = "Transfiriendo al ESP32 (Puede tardar)...";
      
      fetch('/upload', { method: 'POST', body: formData })
      .then(r => r.text())
      .then(txt => {
         status.style.color = "#4CAF50";
         status.innerText = "¡Subida Completada con Éxito!";
         btn.disabled = false;
         document.getElementById('uploadForm').reset();
         setTimeout(() => status.innerText = "", 4000);
      })
      .catch(e => {
         status.style.color = "#ff4444";
         status.innerText = "Error al subir a la tarjeta SD.";
         btn.disabled = false;
      });
    }
  </script>
</body>
</html>
)rawliteral";

void inicializarSistemaWiFi() {
    Serial.println("\n[WiFi] Iniciando encendido...");

    WiFi.mode(WIFI_STA);
    preferences.begin("wifi-config", false);
    String ssidGuardado = preferences.getString("ssid", "");
    String passGuardado = preferences.getString("pass", "");
    preferences.end(); 

    if (ssidGuardado.length() > 0) {
        WiFi.begin(ssidGuardado.c_str(), passGuardado.c_str());
        unsigned long inicio = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - inicio < 6000) {
            delay(100); 
            Serial.print(".");
        }
    }

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("\n[WiFi] Activando Modo Access Point (AP)...");
        WiFi.mode(WIFI_AP); 
        WiFi.softAP("Reproductor_MP3_Config", "12345678");
        modoAPActivo = true;
        Serial.print("[WiFi] IP del Servidor: ");
        Serial.println(WiFi.softAPIP());
    } else {
        modoAPActivo = false;
        Serial.println("\n[WiFi] ¡Conectado exitosamente!");
        Serial.print("[WiFi] IP local: ");
        Serial.println(WiFi.localIP());
    }

    if (!rutasConfiguradas) {
        servidor.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
            request->send_P(200, "text/html", paginaHTML);
        });

        // La ruta de upload ahora responde a peticiones FETCH de JavaScript
        servidor.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request){
            request->send(200, "text/plain", "OK");
        }, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
            static File archivoUpload;
            if(!index){
                String ruta = "/" + filename;
                if(SD.exists(ruta.c_str())) SD.remove(ruta.c_str());
                archivoUpload = SD.open(ruta.c_str(), FILE_WRITE);
                Serial.printf("[SD] Iniciando guardado: %s\n", filename.c_str());
            }
            if(archivoUpload) archivoUpload.write(data, len);
            if(final){
                if(archivoUpload) archivoUpload.close();
                Serial.printf("[SD] ¡Archivo completado!: %s\n", filename.c_str());
            }
        });

        servidor.on("/guardar-wifi", HTTP_POST, [](AsyncWebServerRequest *request){
            if (request->hasParam("ssid", true)) {
                String nuevoSSID = request->getParam("ssid", true)->value();
                String nuevoPass = "";
                if (request->hasParam("password", true)) {
                    nuevoPass = request->getParam("password", true)->value();
                }
                
                preferences.begin("wifi-config", false);
                preferences.putString("ssid", nuevoSSID);
                preferences.putString("pass", nuevoPass);
                preferences.end();

                request->send(200, "text/html", "<h2>Credenciales guardadas. Reiniciando dispositivo...</h2>");
                delay(2000);
                ESP.restart();
            } else {
                request->send(400, "text/plain", "Error en los datos.");
            }
        });

        servidor.on("/update", HTTP_POST, [](AsyncWebServerRequest *request){
            bool updateSuccess = !Update.hasError();
            AsyncWebServerResponse *response = request->beginResponse(200, "text/html", updateSuccess ? "<h2>¡Actualizacion exitosa! Reiniciando...</h2>" : "<h2>Error en la actualizacion.</h2>");
            response->addHeader("Connection", "close");
            request->send(response);
            delay(1000);
            ESP.restart();
        },[](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
            if(!index){
                if(!Update.begin(UPDATE_SIZE_UNKNOWN)) Update.printError(Serial);
            }
            if(len) {
                if(Update.write(data, len) != len) Update.printError(Serial);
            }
            if(final){
                if(Update.end(true)) Serial.printf("[OTA] Exito: %u bytes\n", index+len);
                else Update.printError(Serial);
            }
        });

        rutasConfiguradas = true;
    }

    servidor.begin();
}

void detenerWiFi() {
    servidor.end();
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    Serial.println("[WiFi] Sistema apagado.");
}